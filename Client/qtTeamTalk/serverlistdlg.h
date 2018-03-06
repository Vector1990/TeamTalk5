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

#ifndef SERVERLISTDLG_H
#define SERVERLISTDLG_H

#include "ui_serverlist.h"
#include "common.h"
#include <QVector>
#include <QNetworkAccessManager>

class ServerListDlg : public QDialog
{
    Q_OBJECT
public:
    ServerListDlg(QWidget * parent = 0);

private:
    Ui::ServerListDlg ui;
    void showServers();
    void showLatestHosts();
    QVector<HostEntry> m_servers, m_freeservers;
    QNetworkAccessManager* m_http_manager;

    bool getHostEntry(HostEntry& entry);
    void showHost(const HostEntry& entry);
    void clearServer();

private slots:
    void slotShowHost(int index);
    void slotShowServer(int index);
    void slotAddUpdServer();
    void slotDeleteServer();
    void slotClearServerClicked();
    void slotConnect();
    void slotServerSelected(QListWidgetItem * item);
    void slotDoubleClicked(QListWidgetItem*);
    void slotFreeServers(bool checked);
    void slotFreeServerRequest(QNetworkReply* reply);
    void slotGenerateFile();
    void slotDeleteLatestHost();

    void slotSaveEntryChanged(const QString& text);
    void slotGenerateEntryName(const QString&);
};

#endif
