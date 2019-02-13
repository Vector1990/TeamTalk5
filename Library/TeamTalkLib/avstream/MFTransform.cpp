/*
* Copyright (c) 2005-2018, BearWare.dk
*
* Contact Information:
*
* Bjoern D. Rasmussen
* Kirketoften 5
* DK-8260 Viby J
* Denmark
* Email: contact@bearware.dk
* Phone: +45 20 20 54 59
* Web: http://www.bearware.dk
*
* This source code is part of the TeamTalk SDK owned by
* BearWare.dk. Use of this file, or its compiled unit, requires a
* TeamTalk SDK License Key issued by BearWare.dk.
*
* The TeamTalk SDK License Agreement along with its Terms and
* Conditions are outlined in the file License.txt included with the
* TeamTalk SDK distribution.
*
*/

#include "MFTransform.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <atlbase.h>

#include <assert.h>
#include <complex>
#include <queue>

#define MFT_INIT_FLAGS (MFT_ENUM_FLAG_SORTANDFILTER | MFT_ENUM_FLAG_LOCALMFT | MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_TRANSCODE_ONLY)

class MFTransformImpl : public MFTransform
{
    IMFActivate** m_pMFTs;
    CComPtr<IMFTransform> m_pMFT;
    UINT32 m_cMFTs = 0;
    DWORD m_dwInputID = 0, m_dwOutputID = 0;
    bool m_ready = false;
    mftransform_t m_endpoint;
    media::VideoFormat m_outputvideofmt;
    media::AudioFormat m_outputaudiofmt;
    int m_audio_samples = 0; // samples in audio output buffer

public:
    MFTransformImpl(IMFMediaType* pInputType, const GUID& dest_videoformat, bool decoder, MFTransform* endpoint = nullptr)
    {
        m_endpoint.reset(endpoint);
        HRESULT hr;
        MFT_REGISTER_TYPE_INFO tininfo, toutinfo;

        // get input info
        UINT32 w = 0, h = 0;
        hr = MFGetAttributeSize(pInputType, MF_MT_FRAME_SIZE, &w, &h);
        UINT32 numerator = 0, denominator = 0;
        hr = MFGetAttributeRatio(pInputType, MF_MT_FRAME_RATE, &numerator, &denominator);
        hr = pInputType->GetGUID(MF_MT_MAJOR_TYPE, &tininfo.guidMajorType);
        hr = pInputType->GetGUID(MF_MT_SUBTYPE, &tininfo.guidSubtype);
        assert(tininfo.guidMajorType == MFMediaType_Video);

        // set output info
        toutinfo.guidMajorType = MFMediaType_Video;
        toutinfo.guidSubtype = dest_videoformat;

        m_outputvideofmt = media::VideoFormat(w, h, numerator, denominator, ConvertSubType(dest_videoformat));

        MYTRACE(ACE_TEXT("MFTransform %s -> %s, %dx%d@%u\n"), FourCCToString(ConvertSubType(tininfo.guidSubtype)).c_str(),
            FourCCToString(m_outputvideofmt.fourcc).c_str(), m_outputvideofmt.width, m_outputvideofmt.height, numerator / std::max(denominator, 1u));

        // https://docs.microsoft.com/en-us/windows/desktop/medfound/video-processor-mft
        hr = MFTEnumEx(decoder? MFT_CATEGORY_VIDEO_DECODER : MFT_CATEGORY_VIDEO_PROCESSOR, MFT_INIT_FLAGS, &tininfo, &toutinfo, &m_pMFTs, &m_cMFTs);
        if(FAILED(hr) || m_cMFTs == 0)
            return;

        hr = m_pMFTs[0]->ActivateObject(IID_PPV_ARGS(&m_pMFT));
        if(FAILED(hr))
            return;

        hr = m_pMFT->GetStreamIDs(1, &m_dwInputID, 1, &m_dwOutputID);
        if(hr == E_NOTIMPL)
        {
            // The stream identifiers are zero-based.
            m_dwInputID = 0;
            m_dwOutputID = 0;
            hr = S_OK;
        }
        if(FAILED(hr))
            return;

        hr = m_pMFT->SetInputType(m_dwInputID, pInputType, 0);
        if(FAILED(hr))
            return;

        CComPtr<IMFMediaType> pOutputType;
        hr = MFCreateMediaType(&pOutputType);
        if(FAILED(hr))
            return;

        //hr = pInputType->CopyAllItems(pOutputType);
        //if(FAILED(hr))
        //    return;

        hr = pOutputType->SetGUID(MF_MT_MAJOR_TYPE, toutinfo.guidMajorType);
        if(FAILED(hr))
            return;

        hr = pOutputType->SetGUID(MF_MT_SUBTYPE, toutinfo.guidSubtype);
        if(FAILED(hr))
            return;

        hr = MFSetAttributeSize(pOutputType, MF_MT_FRAME_SIZE, w, h);
        if(FAILED(hr))
            return;

        if (numerator > 0 && denominator > 0)
        {
            hr = MFSetAttributeRatio(pOutputType, MF_MT_FRAME_RATE, numerator, denominator);
            if(FAILED(hr))
                return;
        }

        LONG stride = 0;
        hr = MFGetStrideForBitmapInfoHeader(toutinfo.guidSubtype.Data1, w, &stride);
        if(SUCCEEDED(hr))
        {
            if(stride < 0)
            {
                stride *= -1;
                hr = pOutputType->SetUINT32(MF_MT_DEFAULT_STRIDE, stride);
                assert(SUCCEEDED(hr));
            }
        }

        /*
        UINT32 size = 0;
        hr = MFCalculateImageSize(toutinfo.guidSubtype, w, h, &size);
        if(FAILED(hr))
            return;

        hr = pOutputType->SetUINT32(MF_MT_SAMPLE_SIZE, size);
        if(FAILED(hr))
            return;
        */

        hr = m_pMFT->SetOutputType(m_dwOutputID, pOutputType, 0);
        if(FAILED(hr))
            return;

        hr = m_pMFT->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
        assert(SUCCEEDED(hr));

        hr = m_pMFT->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0);
        assert(SUCCEEDED(hr));

        m_ready = true;
    }

    MFTransformImpl(IMFMediaType* pInputType, IMFMediaType* pOutputType, int nAudioOutputSamples)
    {
        HRESULT hr;
        MFT_REGISTER_TYPE_INFO tininfo, toutinfo;
        DWORD dwTypeIndex = 0;
        CComPtr<IMFMediaType> pTmpMedia;
        UINT32 uOutputSampleRate = 0, uOutputChannels = 0;

        hr = pInputType->GetGUID(MF_MT_MAJOR_TYPE, &tininfo.guidMajorType);
        hr = pInputType->GetGUID(MF_MT_SUBTYPE, &tininfo.guidSubtype);

        // set output info
        hr = pOutputType->GetGUID(MF_MT_MAJOR_TYPE, &toutinfo.guidMajorType);
        hr = pOutputType->GetGUID(MF_MT_SUBTYPE, &toutinfo.guidSubtype);

        hr = MFTEnumEx(MFT_CATEGORY_AUDIO_EFFECT, MFT_INIT_FLAGS, &tininfo, &toutinfo, &m_pMFTs, &m_cMFTs);
        if(FAILED(hr) || m_cMFTs == 0)
            return;

        for (DWORD dwIndex=0;dwIndex<m_cMFTs;dwIndex++)
        {
            m_pMFT.Release();

            hr = m_pMFTs[dwIndex]->ActivateObject(IID_PPV_ARGS(&m_pMFT));
            if(FAILED(hr))
                continue;

            hr = m_pMFT->GetStreamIDs(1, &m_dwInputID, 1, &m_dwOutputID);
            if(hr == E_NOTIMPL)
            {
                // The stream identifiers are zero-based.
                m_dwInputID = 0;
                m_dwOutputID = 0;
                hr = S_OK;
            }
            if(FAILED(hr))
                continue;

            //while (SUCCEEDED(m_pMFT->GetOutputAvailableType(m_dwOutputID, dwTypeIndex, &pTmpMedia)))
            //{
            //    DWORD dwFlags = 0;
            //    dwTypeIndex++;
            //    hr = pTmpMedia->IsEqual(pOutputType, &dwFlags);
            //    pTmpMedia.Release();
            //}

            hr = m_pMFT->SetInputType(m_dwInputID, pInputType, 0);
            if(FAILED(hr))
                continue;

            hr = m_pMFT->SetOutputType(m_dwOutputID, pOutputType, 0);
            if(FAILED(hr))
                continue;

            break;
        }

        if (FAILED(hr))
            return;

        hr = pOutputType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &uOutputChannels);
        if(FAILED(hr))
            return;

        hr = pOutputType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &uOutputSampleRate);
        if(FAILED(hr))
            return;

        m_outputaudiofmt = media::AudioFormat(int(uOutputSampleRate), int(uOutputChannels));
        m_audio_samples = nAudioOutputSamples;

        assert(m_outputaudiofmt.IsValid());

        hr = m_pMFT->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
        assert(SUCCEEDED(hr));

        hr = m_pMFT->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0);
        assert(SUCCEEDED(hr));

        m_ready = true;
    }


    ~MFTransformImpl()
    {
        if(m_cMFTs)
            CoTaskMemFree(m_pMFTs);
    }

    bool Ready() const { return m_ready; }

    TransformState SubmitSample(CComPtr<IMFSample>& pInSample)
    {
        assert(Ready());
        
        HRESULT hr;
        hr = m_pMFT->ProcessInput(m_dwInputID, pInSample, 0);
        MYTRACE_COND(hr == MF_E_NOTACCEPTING, ACE_TEXT("Now enough input to generate output\n"));
        switch(hr)
        {
        case MF_E_NOTACCEPTING :
            return TRANSFORM_INPUT_BLOCKED; // output ready, so cannot submit
        case E_INVALIDARG :
            assert(hr != E_INVALIDARG);
            return TRANSFORM_ERROR;
        case MF_E_INVALIDSTREAMNUMBER :
            assert(hr != MF_E_INVALIDSTREAMNUMBER);
            return TRANSFORM_ERROR;
        case MF_E_NO_SAMPLE_DURATION :
            assert(hr != MF_E_NO_SAMPLE_DURATION);
            return TRANSFORM_ERROR;
        case MF_E_NO_SAMPLE_TIMESTAMP :
            assert(hr != MF_E_NO_SAMPLE_TIMESTAMP);
            return TRANSFORM_ERROR;
        case MF_E_TRANSFORM_TYPE_NOT_SET :
            assert(hr != MF_E_TRANSFORM_TYPE_NOT_SET);
            return TRANSFORM_ERROR;
        case MF_E_UNSUPPORTED_D3D_TYPE :
            assert(hr != MF_E_UNSUPPORTED_D3D_TYPE);
            return TRANSFORM_ERROR;
        case S_OK :
        {
            DWORD dwFlags;
            hr = m_pMFT->GetOutputStatus(&dwFlags);
            if (SUCCEEDED(hr) && dwFlags == MFT_OUTPUT_STATUS_SAMPLE_READY)
                return TRANSFORM_IO_SUCCESS;
            return TRANSFORM_SUBMITTED;
        }
        default :
            assert(hr == S_OK);
            return TRANSFORM_ERROR;
        }
    }

    TransformState SubmitSample(const media::VideoFrame& frame)
    {
        HRESULT hr;
        CComPtr<IMFSample> pSample;
        CComPtr<IMFMediaBuffer> pMediaBuffer;

        hr = MFCreateSample(&pSample);
        if(FAILED(hr))
            goto fail;

        hr = pSample->SetSampleTime(frame.timestamp * 10000);
        if(FAILED(hr))
            goto fail;

        //hr = pSample->SetSampleDuration((1000 / 30) * 10000);
        //if(FAILED(hr))
        //    goto fail;

        hr = MFCreateMemoryBuffer(frame.frame_length, &pMediaBuffer);
        if(FAILED(hr))
            goto fail;

        BYTE* pBuffer;
        DWORD dwCurLen, dwMaxSize;
        hr = pMediaBuffer->Lock(&pBuffer, &dwMaxSize, &dwCurLen);
        assert(SUCCEEDED(hr));
        if(FAILED(hr))
            goto fail;

        assert(dwMaxSize == frame.frame_length);
        memcpy_s(pBuffer, dwMaxSize, frame.frame, frame.frame_length);
        hr = pMediaBuffer->SetCurrentLength(frame.frame_length);
        assert(SUCCEEDED(hr));
        if(FAILED(hr))
            goto fail;

        hr = pMediaBuffer->Unlock();
        assert(SUCCEEDED(hr));
        if(FAILED(hr))
            goto fail;

        hr = pSample->AddBuffer(pMediaBuffer);
        assert(SUCCEEDED(hr));
        if(FAILED(hr))
            goto fail;

        return SubmitSample(pSample);

    fail:
        return TRANSFORM_ERROR;
    }

    TransformState SubmitSample(const media::AudioFrame& frame)
    {
        HRESULT hr;
        CComPtr<IMFSample> pSample;
        CComPtr<IMFMediaBuffer> pMediaBuffer;
        DWORD dwBufSize = PCM16_BYTES(frame.input_samples, frame.inputfmt.channels);

        hr = MFCreateSample(&pSample);
        if(FAILED(hr))
            goto fail;

        //hr = pSample->SetSampleTime(frame.timestamp * 10000);
        //if(FAILED(hr))
        //    goto fail;

        //hr = pSample->SetSampleDuration((1000 / 30) * 10000);
        //if(FAILED(hr))
        //    goto fail;

        hr = MFCreateMemoryBuffer(dwBufSize, &pMediaBuffer);
        if(FAILED(hr))
            goto fail;

        BYTE* pBuffer;
        DWORD dwCurLen, dwMaxSize;
        hr = pMediaBuffer->Lock(&pBuffer, &dwMaxSize, &dwCurLen);
        assert(SUCCEEDED(hr));
        if(FAILED(hr))
            goto fail;

        assert(dwMaxSize == dwBufSize);
        memcpy_s(pBuffer, dwMaxSize, frame.input_buffer, dwBufSize);
        hr = pMediaBuffer->SetCurrentLength(dwBufSize);
        assert(SUCCEEDED(hr));
        if(FAILED(hr))
            goto fail;

        hr = pMediaBuffer->Unlock();
        assert(SUCCEEDED(hr));
        if(FAILED(hr))
            goto fail;

        hr = pSample->AddBuffer(pMediaBuffer);
        assert(SUCCEEDED(hr));
        if(FAILED(hr))
            goto fail;

        return SubmitSample(pSample);

    fail:
        return TRANSFORM_ERROR;
    }

    std::vector< CComPtr<IMFSample> > RetrieveSample()
    {
        assert(Ready());

        HRESULT hr;

        DWORD dwStatus;

        std::vector< CComPtr<IMFSample> > outputsamples;
        
        // This check does not always yield correct result.
        //if (SUCCEEDED(m_pMFT->GetOutputStatus(&dwFlags)) && dwFlags != MFT_OUTPUT_STATUS_SAMPLE_READY)
        //    return imfsamples_t();

        MFT_OUTPUT_STREAM_INFO mftStreamInfo = {};
        hr = m_pMFT->GetOutputStreamInfo(m_dwOutputID, &mftStreamInfo);
        if (FAILED(hr))
            return imfsamples_t();

        do
        {
            MFT_OUTPUT_DATA_BUFFER mftOutputData = {};
            CComPtr<IMFSample> pOutSample;

            mftOutputData.dwStreamID = m_dwOutputID;
            mftOutputData.dwStatus = 0;
            mftOutputData.pEvents = nullptr;

            if(mftStreamInfo.dwFlags & (MFT_OUTPUT_STREAM_PROVIDES_SAMPLES | MFT_OUTPUT_STREAM_CAN_PROVIDE_SAMPLES))
            {
                mftOutputData.pSample = nullptr;
            }
            else
            {
                DWORD dwBufSize = mftStreamInfo.cbSize;
                if (m_audio_samples)
                {
                    assert(m_outputaudiofmt.IsValid());
                    dwBufSize = PCM16_BYTES(m_audio_samples, m_outputaudiofmt.channels);
                }

                hr = MFCreateSample(&pOutSample);
                if(FAILED(hr))
                    return imfsamples_t();

                CComPtr<IMFMediaBuffer> pBufferOut;
                if(mftStreamInfo.cbAlignment)
                {
                    hr = MFCreateAlignedMemoryBuffer(dwBufSize, mftStreamInfo.cbAlignment - 1, &pBufferOut);
                }
                else
                {
                    hr = MFCreateMemoryBuffer(dwBufSize, &pBufferOut);
                }
                assert(SUCCEEDED(hr));
                if(FAILED(hr))
                    return imfsamples_t();

                hr = pOutSample->AddBuffer(pBufferOut);
                if(FAILED(hr))
                    return imfsamples_t();

                mftOutputData.pSample = pOutSample;
            }

            hr = m_pMFT->ProcessOutput(0, 1, &mftOutputData, &dwStatus);

            if(mftOutputData.pEvents)
                mftOutputData.pEvents->Release();

            switch(hr)
            {
            case E_UNEXPECTED:
                assert(hr != E_UNEXPECTED);
                return imfsamples_t();
            case MF_E_INVALIDSTREAMNUMBER:
                assert(hr != MF_E_INVALIDSTREAMNUMBER);
                return imfsamples_t();
            case MF_E_TRANSFORM_STREAM_CHANGE:
                assert(hr != MF_E_TRANSFORM_STREAM_CHANGE);
                return imfsamples_t();
            case MF_E_TRANSFORM_TYPE_NOT_SET:
                assert(hr != MF_E_TRANSFORM_TYPE_NOT_SET);
                return imfsamples_t();
            case S_OK:
                if (mftOutputData.pSample)
                    outputsamples.push_back(mftOutputData.pSample);
                break;
            case MF_E_TRANSFORM_NEED_MORE_INPUT:
                break;
            default:
                assert(hr == S_OK);
                return imfsamples_t();
            }
        }
        while (hr != MF_E_TRANSFORM_NEED_MORE_INPUT);

        // Flushing the transformer seems to drop mpeg packets so no output is ever generated.

        //hr = m_pMFT->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, m_dwInputID);
        //if(FAILED(hr))
        //    return imfsamples_t();

        if (m_endpoint.get())
        {
            std::vector< CComPtr<IMFSample> > endpointsamples;

            for (auto& pSample : outputsamples)
            {
                auto endpointresult = m_endpoint->ProcessSample(pSample);
                endpointsamples.insert(endpointsamples.end(), endpointresult.begin(), endpointresult.end());
            }
            outputsamples = endpointsamples;
        }

        return outputsamples;
    }

    std::vector<ACE_Message_Block*> RetrieveMBSample()
    {
        std::vector<ACE_Message_Block*> result;

        std::vector< CComPtr<IMFSample> > samples = RetrieveSample();
        for (auto& pSample: samples)
        {
            assert(pSample);
            if(!pSample.p)
                continue;

            if(m_outputvideofmt.IsValid())
                result.push_back(ConvertVideoSample(pSample, m_outputvideofmt));

            if(m_outputaudiofmt.IsValid())
                result.push_back(ConvertAudioSample(pSample, m_outputaudiofmt));
        }
        return result;
    }

    std::vector< CComPtr<IMFSample> > ProcessSample(CComPtr<IMFSample>& pInSample)
    {
        imfsamples_t result = RetrieveSample();

        UINT32 uTransform = SubmitSample(pInSample);
        switch(uTransform)
        {
            // input block, try draining and resubmit
        case TRANSFORM_INPUT_BLOCKED:
        {
            auto samples = RetrieveSample();
            result.insert(result.end(), samples.begin(), samples.end());
            uTransform = SubmitSample(pInSample);
            switch(uTransform)
            {
            case TRANSFORM_IO_SUCCESS:
            {
                samples = RetrieveSample();
                result.insert(result.end(), samples.begin(), samples.end());
                MYTRACE_COND(result.empty(), ACE_TEXT("Transform output reported but no samples retrieved.\n"));
                break;
            }
            case TRANSFORM_SUBMITTED:
                break;
            default:
                assert(0);
            }
            break;
        }
            // input success and output ready
        case TRANSFORM_IO_SUCCESS:
        {
            auto samples = RetrieveSample();
            result.insert(result.end(), samples.begin(), samples.end());
            assert(result.size());
            //MYTRACE(ACE_TEXT("Got frame from I/O success: %d\n"), ++frames);
            break;
        }
        default:
            MYTRACE_COND(uTransform & TRANSFORM_SUBMITTED, ACE_TEXT("Submitted but no output\n"));
            MYTRACE_COND(uTransform & TRANSFORM_ERROR, ACE_TEXT("Transform error\n"));
            break;
        }

        return result;
    }

    std::vector< ACE_Message_Block* > ProcessMBSample(CComPtr<IMFSample>& pInSample)
    {
        std::vector< ACE_Message_Block* > result;

        auto outputsamples = ProcessSample(pInSample);

        for (auto& pOutSample : outputsamples)
        {
            if(!pOutSample)
                continue;

            if(m_outputvideofmt.IsValid())
                result.push_back(ConvertVideoSample(pOutSample, m_outputvideofmt));

            if(m_outputaudiofmt.IsValid())
                result.push_back(ConvertAudioSample(pOutSample, m_outputaudiofmt));
        }

        return result;
    }


};

mftransform_t MFTransform::Create(IMFMediaType* pInputType, const GUID& dest_videoformat)
{
    std::unique_ptr<MFTransformImpl> result;
    result.reset(new MFTransformImpl(pInputType, dest_videoformat, false));
    if (!result->Ready())
    {
        result.reset();

        // try creating an intermediate transformer (endpoint)
        // using a decoder instead of processor
        // (MFT_CATEGORY_VIDEO_PROCESSOR vs MFT_CATEGORY_VIDEO_DECODER)
        HRESULT hr;
        MFT_REGISTER_TYPE_INFO tininfo;
        tininfo.guidMajorType = MFMediaType_Video;
        hr = pInputType->GetGUID(MF_MT_SUBTYPE, &tininfo.guidSubtype);
        if (FAILED(hr))
            return result;

        IMFActivate** pMFTs;
        UINT32 cMFTs = 0;
        hr = MFTEnumEx(MFT_CATEGORY_VIDEO_DECODER, MFT_INIT_FLAGS, &tininfo, NULL, &pMFTs, &cMFTs);

        for (UINT32 i = 0; i < cMFTs; ++i)
        {
            CComPtr<IMFTransform> pMFT;

            hr = pMFTs[0]->ActivateObject(IID_PPV_ARGS(&pMFT));
            if (FAILED(hr))
                continue;
            
            DWORD dwInputID = 0, dwOutputID = 0;
            hr = pMFT->SetInputType(dwInputID, pInputType, 0);
            if(FAILED(hr))
                continue;

            DWORD dwIndex = 0;
            CComPtr<IMFMediaType> pOutputType;
            while(SUCCEEDED(pMFT->GetOutputAvailableType(dwOutputID, dwIndex, &pOutputType)))
            {
                GUID subType;
                hr = pOutputType->GetGUID(MF_MT_SUBTYPE, &subType);
                if (SUCCEEDED(hr))
                {
                    mftransform_t endpoint = MFTransform::Create(pOutputType, dest_videoformat);
                    if(endpoint.get())
                    {
                        result.reset(new MFTransformImpl(pInputType, subType, true, endpoint.release()));
                        if(result->Ready())
                            return result;
                    }
                }
                pOutputType.Release();
                dwIndex++;
            }
        }
    }
    return result;
}

mftransform_t MFTransform::Create(const media::VideoFormat& inputfmt, media::FourCC outputfmt)
{
    std::unique_ptr<MFTransform> result;

    HRESULT hr;
    CComPtr<IMFMediaType> pInputType;
    LONG stride = 0;
    hr = MFCreateMediaType(&pInputType);
    if (FAILED(hr))
        goto fail;

    pInputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    GUID subType = ConvertFourCC(inputfmt.fourcc);
    if(subType == GUID_NULL)
        goto fail;
    hr = pInputType->SetGUID(MF_MT_SUBTYPE, subType);
    if(FAILED(hr))
        goto fail;
    
    hr = MFGetStrideForBitmapInfoHeader(subType.Data1, inputfmt.width, &stride);
    if(FAILED(hr))
        goto fail;

    hr = MFSetAttributeRatio(pInputType, MF_MT_FRAME_RATE, inputfmt.fps_numerator, inputfmt.fps_denominator);
    if(FAILED(hr))
        goto fail;

    hr = MFSetAttributeSize(pInputType, MF_MT_FRAME_SIZE, inputfmt.width, inputfmt.height);
    if(FAILED(hr))
        goto fail;

    //hr = pInputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
    //if(FAILED(hr))
    //    goto fail;

    //hr = pInputType->SetUINT32(MF_MT_INTERLACE_MODE, 2);
    //if(FAILED(hr))
    //    goto fail;

    //hr = MFSetAttributeRatio(pInputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
    //if(FAILED(hr))
    //    goto fail;

    //hr = MFCalculateImageSize(subType, inputfmt.width, inputfmt.height, &size);
    //if(FAILED(hr))
    //    goto fail;

    //hr = pInputType->SetUINT32(MF_MT_SAMPLE_SIZE, size);
    //if(FAILED(hr))
    //    goto fail;

    result = MFTransform::Create(pInputType, ConvertFourCC(outputfmt));

    fail:
    return result;
}

mftransform_t MFTransform::Create(media::AudioFormat inputfmt, media::AudioFormat outputfmt, int output_samples)
{
    std::unique_ptr<MFTransformImpl> result;

    HRESULT hr;
    CComPtr<IMFMediaType> pInputType, pOutputType;

    hr = MFCreateMediaType(&pInputType);
    if(FAILED(hr))
        goto fail;

    hr = pInputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    if(FAILED(hr))
        goto fail;

    hr = pInputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    if(FAILED(hr))
        goto fail;

    hr = pInputType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, inputfmt.channels);
    if(FAILED(hr))
        goto fail;

    hr = pInputType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, inputfmt.samplerate);
    if(FAILED(hr))
        goto fail;

    hr = pInputType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, PCM16_BYTES(inputfmt.samplerate, inputfmt.channels));
    if(FAILED(hr))
        goto fail;

    hr = pInputType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, PCM16_BYTES(1, inputfmt.channels));
    if(FAILED(hr))
        goto fail;

    hr = pInputType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
    if(FAILED(hr))
        goto fail;
    
    hr = pInputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
    if(FAILED(hr))
        goto fail;
    


    hr = MFCreateMediaType(&pOutputType);
    if(FAILED(hr))
        goto fail;

    hr = pOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    if(FAILED(hr))
        goto fail;

    hr = pOutputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    if(FAILED(hr))
        goto fail;

    hr = pOutputType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, outputfmt.channels);
    if(FAILED(hr))
        goto fail;

    hr = pOutputType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, outputfmt.samplerate);
    if(FAILED(hr))
        goto fail;

    hr = pOutputType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, PCM16_BYTES(outputfmt.samplerate, outputfmt.channels));
    if(FAILED(hr))
        goto fail;

    hr = pOutputType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, PCM16_BYTES(1, outputfmt.channels));
    if(FAILED(hr))
        goto fail;

    hr = pOutputType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
    if(FAILED(hr))
        goto fail;

    hr = pOutputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
    if(FAILED(hr))
        goto fail;


    result.reset(new MFTransformImpl(pInputType, pOutputType, output_samples));

    if (!result->Ready())
        result.reset();

fail:
    return result;
}



media::FourCC ConvertSubType(const GUID& native_subtype)
{
    if(native_subtype == MFVideoFormat_RGB32)
    {
        return media::FOURCC_RGB32;
    }
    else if(native_subtype == MFVideoFormat_ARGB32)
    {
        return media::FOURCC_RGB32;
    }
    else if(native_subtype == MFVideoFormat_I420)
    {
        return media::FOURCC_I420;
    }
    else if(native_subtype == MFVideoFormat_YUY2)
    {
        return media::FOURCC_YUY2;
    }
    else if(native_subtype == MFVideoFormat_H264)
    {
        return media::FOURCC_H264;
    }
    else if(native_subtype == MFVideoFormat_RGB24)
    {
        return media::FOURCC_RGB24;
    }
    else if(native_subtype == MFVideoFormat_NV12)
    {
        return media::FOURCC_NV12;
    }
    else if(native_subtype == MFVideoFormat_MJPG)
    {
        return media::FOURCC_MJPEG;
    }
    else
    {
        return media::FOURCC_NONE;
    }
}

const GUID& ConvertFourCC(media::FourCC fcc)
{
    switch(fcc)
    {
    case media::FOURCC_RGB32:
        return MFVideoFormat_ARGB32;
    case media::FOURCC_I420:
        return MFVideoFormat_I420;
    case media::FOURCC_YUY2:
        return MFVideoFormat_YUY2;
    case media::FOURCC_RGB24:
        return MFVideoFormat_RGB24;
    case media::FOURCC_NV12:
        return MFVideoFormat_NV12;
    case media::FOURCC_H264:
        return MFVideoFormat_H264;
    case media::FOURCC_MJPEG:
        return MFVideoFormat_MJPG;
    default:
        return GUID_NULL;
    }
}

ACE_TString FourCCToString(media::FourCC fcc)
{
    switch(fcc)
    {
    case media::FOURCC_RGB32:
        return ACE_TEXT("RGB32");
    case media::FOURCC_I420:
        return ACE_TEXT("I420");
    case media::FOURCC_YUY2:
        return ACE_TEXT("YUY2");
    case media::FOURCC_RGB24:
        return ACE_TEXT("RGB24");
    case media::FOURCC_NV12:
        return ACE_TEXT("NV12");
    case media::FOURCC_H264:
        return ACE_TEXT("H264");
    case media::FOURCC_MJPEG:
        return ACE_TEXT("MJPEG");
    default:
        return ACE_TEXT("NONE");
    }
}

media::VideoFormat ConvertMediaType(IMFMediaType* pInputType)
{
    HRESULT hr;
    GUID major, subtype;
    UINT32 w = 0, h = 0;
    UINT32 numerator = 0, denominator = 0;

    hr = pInputType->GetGUID(MF_MT_MAJOR_TYPE, &major);
    if (FAILED(hr) || major != MFMediaType_Video)
        return media::VideoFormat();

    hr = pInputType->GetGUID(MF_MT_SUBTYPE, &subtype);
    if (FAILED(hr))
        return media::VideoFormat();

    hr = MFGetAttributeSize(pInputType, MF_MT_FRAME_SIZE, &w, &h);
    if(FAILED(hr))
        return media::VideoFormat();

    hr = MFGetAttributeRatio(pInputType, MF_MT_FRAME_RATE, &numerator, &denominator);
    
    return media::VideoFormat(w, h, numerator, denominator, ConvertSubType(subtype));
}

ACE_Message_Block* ConvertVideoSample(IMFSample* pSample, const media::VideoFormat& fmt)
{
    HRESULT hr;
    ACE_UINT32 timestamp = 0;
    DWORD dwBufCount = 0;
    ACE_Message_Block* mb = nullptr;
    assert(fmt.IsValid());
    if(pSample)
    {
        hr = pSample->GetBufferCount(&dwBufCount);
        assert(SUCCEEDED(hr));
        LONGLONG llSampleTime = 0;
        if(SUCCEEDED(pSample->GetSampleTime(&llSampleTime)))
            timestamp = ACE_UINT32(llSampleTime / 10000);
    }

    for(DWORD i = 0; i<dwBufCount && mb == nullptr; i++)
    {
        CComPtr<IMFMediaBuffer> pMediaBuffer;
        hr = pSample->GetBufferByIndex(i, &pMediaBuffer);
        assert(SUCCEEDED(hr));
        if(FAILED(hr))
            break;

        BYTE* pBuffer = NULL;
        DWORD dwCurLen, dwMaxSize;
        hr = pMediaBuffer->Lock(&pBuffer, &dwMaxSize, &dwCurLen);
        assert(SUCCEEDED(hr));
        if(SUCCEEDED(hr) && dwCurLen > 0)
        {
            media::VideoFrame frame(fmt, reinterpret_cast<char*>(pBuffer), dwCurLen);
            frame.timestamp = timestamp;
            mb = VideoFrameToMsgBlock(frame);
        }
        hr = pMediaBuffer->Unlock();
        assert(SUCCEEDED(hr));
    }
    return mb;
}


ACE_Message_Block* ConvertAudioSample(IMFSample* pSample, const media::AudioFormat& fmt)
{
    HRESULT hr;
    ACE_UINT32 timestamp = 0;
    DWORD dwBufCount = 0;
    ACE_Message_Block* mb = nullptr;
    assert(fmt.IsValid());
    if(pSample)
    {
        hr = pSample->GetBufferCount(&dwBufCount);
        assert(SUCCEEDED(hr));
        LONGLONG llSampleTime = 0;
        if(SUCCEEDED(pSample->GetSampleTime(&llSampleTime)))
            timestamp = ACE_UINT32(llSampleTime / 10000);
    }

    for(DWORD i = 0; i<dwBufCount && mb == nullptr; i++)
    {
        assert(dwBufCount == 1);

        CComPtr<IMFMediaBuffer> pMediaBuffer;
        hr = pSample->GetBufferByIndex(i, &pMediaBuffer);
        assert(SUCCEEDED(hr));
        if(FAILED(hr))
            break;

        BYTE* pBuffer = NULL;
        DWORD dwCurLen, dwMaxSize;
        hr = pMediaBuffer->Lock(&pBuffer, &dwMaxSize, &dwCurLen);
        assert(SUCCEEDED(hr));
        if(SUCCEEDED(hr) && dwCurLen>0)
        {
            media::AudioFrame frame;
            frame.inputfmt = fmt;
            frame.input_buffer = reinterpret_cast<short*>(pBuffer);
            frame.input_samples = dwCurLen / PCM16_BYTES(1, fmt.channels);
            frame.timestamp = timestamp;
            mb = AudioFrameToMsgBlock(frame);
        }
        hr = pMediaBuffer->Unlock();
        assert(SUCCEEDED(hr));
    }
    return mb;
}
