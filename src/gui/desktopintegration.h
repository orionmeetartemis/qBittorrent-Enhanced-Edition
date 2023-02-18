/*
 * Bittorrent Client using Qt and libtorrent.
 * Copyright (C) 2022  Vladimir Golovnev <glassez@yandex.ru>
 * Copyright (C) 2006  Christophe Dumez <chris@qbittorrent.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * In addition, as a special exception, the copyright holders give permission to
 * link this program with the OpenSSL project's "OpenSSL" library (or with
 * modified versions of it that use the same license as the "OpenSSL" library),
 * and distribute the linked executables. You must obey the GNU General Public
 * License in all respects for all of the code used other than "OpenSSL".  If you
 * modify file(s), you may extend this exception to your version of the file(s),
 * but you are not obligated to do so. If you do not wish to do so, delete this
 * exception statement from your version.
 */

#pragma once

#include <QObject>

#include "base/settingvalue.h"

class QMenu;
#ifndef Q_OS_MACOS
class QSystemTrayIcon;
#endif
#if (defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)) && defined(QT_DBUS_LIB)
#define QBT_USES_CUSTOMDBUSNOTIFICATIONS
class DBusNotifier;
#endif

class DesktopIntegration final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(DesktopIntegration)

public:
    explicit DesktopIntegration(QObject *parent = nullptr);
    ~DesktopIntegration() override;

    bool isActive() const;

    QString toolTip() const;
    void setToolTip(const QString &toolTip);

    QMenu *menu() const;
    void setMenu(QMenu *menu);

    bool isNotificationsEnabled() const;
    void setNotificationsEnabled(bool value);

    int notificationTimeout() const;
#ifdef QBT_USES_CUSTOMDBUSNOTIFICATIONS
    void setNotificationTimeout(const int value);
#endif

    void showNotification(const QString &title, const QString &msg) const;

signals:
    void activationRequested();
    void notificationClicked();
    void stateChanged();

private:
    void onPreferencesChanged();
#ifndef Q_OS_MACOS
    void createTrayIcon();
#endif // Q_OS_MACOS

    CachedSettingValue<bool> m_storeNotificationEnabled;

    QMenu *m_menu = nullptr;
    QString m_toolTip;
#ifndef Q_OS_MACOS
    QSystemTrayIcon *m_systrayIcon = nullptr;
#endif
#ifdef QBT_USES_CUSTOMDBUSNOTIFICATIONS
    CachedSettingValue<int> m_storeNotificationTimeOut;
    DBusNotifier *m_notifier = nullptr;
#endif
};
