/*
 * Bittorrent Client using Qt and libtorrent.
 * Copyright (C) 2015-2023  Vladimir Golovnev <glassez@yandex.ru>
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

#include "addtorrentparams.h"

#include <tuple>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include "base/utils/string.h"

const QString PARAM_CATEGORY = u"category"_s;
const QString PARAM_TAGS = u"tags"_s;
const QString PARAM_SAVEPATH = u"save_path"_s;
const QString PARAM_USEDOWNLOADPATH = u"use_download_path"_s;
const QString PARAM_DOWNLOADPATH = u"download_path"_s;
const QString PARAM_OPERATINGMODE = u"operating_mode"_s;
const QString PARAM_QUEUETOP = u"add_to_top_of_queue"_s;
const QString PARAM_STOPPED = u"stopped"_s;
const QString PARAM_STOPCONDITION = u"stop_condition"_s;
const QString PARAM_SKIPCHECKING = u"skip_checking"_s;
const QString PARAM_CONTENTLAYOUT = u"content_layout"_s;
const QString PARAM_AUTOTMM = u"use_auto_tmm"_s;
const QString PARAM_UPLOADLIMIT = u"upload_limit"_s;
const QString PARAM_DOWNLOADLIMIT = u"download_limit"_s;
const QString PARAM_SEEDINGTIMELIMIT = u"seeding_time_limit"_s;
const QString PARAM_INACTIVESEEDINGTIMELIMIT = u"inactive_seeding_time_limit"_s;
const QString PARAM_RATIOLIMIT = u"ratio_limit"_s;

namespace
{
    TagSet parseTagSet(const QJsonArray &jsonArr)
    {
        TagSet tags;
        for (const QJsonValue &jsonVal : jsonArr)
            tags.insert(jsonVal.toString());

        return tags;
    }

    QJsonArray serializeTagSet(const TagSet &tags)
    {
        QJsonArray arr;
        for (const QString &tag : tags)
            arr.append(tag);

        return arr;
    }

    std::optional<bool> getOptionalBool(const QJsonObject &jsonObj, const QString &key)
    {
        const QJsonValue jsonVal = jsonObj.value(key);
        if (jsonVal.isUndefined() || jsonVal.isNull())
            return std::nullopt;

        return jsonVal.toBool();
    }

    template <typename Enum>
    std::optional<Enum> getOptionalEnum(const QJsonObject &jsonObj, const QString &key)
    {
        const QJsonValue jsonVal = jsonObj.value(key);
        if (jsonVal.isUndefined() || jsonVal.isNull())
            return std::nullopt;

        return Utils::String::toEnum<Enum>(jsonVal.toString(), {});
    }

    template <typename Enum>
    Enum getEnum(const QJsonObject &jsonObj, const QString &key)
    {
        const QJsonValue jsonVal = jsonObj.value(key);
        return Utils::String::toEnum<Enum>(jsonVal.toString(), {});
    }
}

bool BitTorrent::operator==(const AddTorrentParams &lhs, const AddTorrentParams &rhs)
{
        return std::tie(lhs.name, lhs.category, lhs.tags,
                lhs.savePath, lhs.useDownloadPath, lhs.downloadPath,
                lhs.sequential, lhs.firstLastPiecePriority, lhs.addForced,
                lhs.addToQueueTop, lhs.addPaused, lhs.stopCondition,
                lhs.filePaths, lhs.filePriorities, lhs.skipChecking,
                lhs.contentLayout, lhs.useAutoTMM, lhs.uploadLimit,
                lhs.downloadLimit, lhs.seedingTimeLimit, lhs.inactiveSeedingTimeLimit, lhs.ratioLimit)
            == std::tie(rhs.name, rhs.category, rhs.tags,
                rhs.savePath, rhs.useDownloadPath, rhs.downloadPath,
                rhs.sequential, rhs.firstLastPiecePriority, rhs.addForced,
                rhs.addToQueueTop, rhs.addPaused, rhs.stopCondition,
                rhs.filePaths, rhs.filePriorities, rhs.skipChecking,
                rhs.contentLayout, rhs.useAutoTMM, rhs.uploadLimit,
                rhs.downloadLimit, rhs.seedingTimeLimit, rhs.inactiveSeedingTimeLimit, rhs.ratioLimit);
}

BitTorrent::AddTorrentParams BitTorrent::parseAddTorrentParams(const QJsonObject &jsonObj)
{
    AddTorrentParams params;
    params.category = jsonObj.value(PARAM_CATEGORY).toString();
    params.tags = parseTagSet(jsonObj.value(PARAM_TAGS).toArray());
    params.savePath = Path(jsonObj.value(PARAM_SAVEPATH).toString());
    params.useDownloadPath = getOptionalBool(jsonObj, PARAM_USEDOWNLOADPATH);
    params.downloadPath = Path(jsonObj.value(PARAM_DOWNLOADPATH).toString());
    params.addForced = (getEnum<TorrentOperatingMode>(jsonObj, PARAM_OPERATINGMODE) == TorrentOperatingMode::Forced);
    params.addToQueueTop = getOptionalBool(jsonObj, PARAM_QUEUETOP);
    params.addPaused = getOptionalBool(jsonObj, PARAM_STOPPED);
    params.stopCondition = getOptionalEnum<Torrent::StopCondition>(jsonObj, PARAM_STOPCONDITION);
    params.skipChecking = jsonObj.value(PARAM_SKIPCHECKING).toBool();
    params.contentLayout = getOptionalEnum<TorrentContentLayout>(jsonObj, PARAM_CONTENTLAYOUT);
    params.useAutoTMM = getOptionalBool(jsonObj, PARAM_AUTOTMM);
    params.uploadLimit = jsonObj.value(PARAM_UPLOADLIMIT).toInt(-1);
    params.downloadLimit = jsonObj.value(PARAM_DOWNLOADLIMIT).toInt(-1);
    params.seedingTimeLimit = jsonObj.value(PARAM_SEEDINGTIMELIMIT).toInt(Torrent::USE_GLOBAL_SEEDING_TIME);
    params.inactiveSeedingTimeLimit = jsonObj.value(PARAM_INACTIVESEEDINGTIMELIMIT).toInt(Torrent::USE_GLOBAL_INACTIVE_SEEDING_TIME);
    params.ratioLimit = jsonObj.value(PARAM_RATIOLIMIT).toDouble(Torrent::USE_GLOBAL_RATIO);

    return params;
}

QJsonObject BitTorrent::serializeAddTorrentParams(const AddTorrentParams &params)
{
    QJsonObject jsonObj {
        {PARAM_CATEGORY, params.category},
        {PARAM_TAGS, serializeTagSet(params.tags)},
        {PARAM_SAVEPATH, params.savePath.data()},
        {PARAM_DOWNLOADPATH, params.downloadPath.data()},
        {PARAM_OPERATINGMODE, Utils::String::fromEnum(params.addForced
                                                          ? TorrentOperatingMode::Forced : TorrentOperatingMode::AutoManaged)},
        {PARAM_SKIPCHECKING, params.skipChecking},
        {PARAM_UPLOADLIMIT, params.uploadLimit},
        {PARAM_DOWNLOADLIMIT, params.downloadLimit},
        {PARAM_SEEDINGTIMELIMIT, params.seedingTimeLimit},
        {PARAM_INACTIVESEEDINGTIMELIMIT, params.inactiveSeedingTimeLimit},
        {PARAM_RATIOLIMIT, params.ratioLimit}
    };

    if (params.addToQueueTop)
        jsonObj[PARAM_QUEUETOP] = *params.addToQueueTop;
    if (params.addPaused)
        jsonObj[PARAM_STOPPED] = *params.addPaused;
    if (params.stopCondition)
        jsonObj[PARAM_STOPCONDITION] = Utils::String::fromEnum(*params.stopCondition);
    if (params.contentLayout)
        jsonObj[PARAM_CONTENTLAYOUT] = Utils::String::fromEnum(*params.contentLayout);
    if (params.useAutoTMM)
        jsonObj[PARAM_AUTOTMM] = *params.useAutoTMM;
    if (params.useDownloadPath)
        jsonObj[PARAM_USEDOWNLOADPATH] = *params.useDownloadPath;

    return jsonObj;
}
