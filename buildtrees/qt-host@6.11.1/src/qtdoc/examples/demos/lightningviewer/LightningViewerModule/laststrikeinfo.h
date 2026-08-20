// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef LASTSTRIKEINFO_H
#define LASTSTRIKEINFO_H

#include <chrono>


struct LastStrikeInfo
{
    double distance {-1};
    std::chrono::duration<int> timestamp {0};
    double direction {0};

    LastStrikeInfo() = default;
    inline LastStrikeInfo(double distance, std::chrono::duration<int> timestamp, double direction);

    void invalidate() { distance = -1; }
    bool isValid() const { return !(distance < 0.0); }
    inline bool operator<(const LastStrikeInfo &other) const;
};


LastStrikeInfo::LastStrikeInfo(double distance,
                               std::chrono::duration<int> timestamp,
                               double direction)
    : distance(distance)
    , timestamp(timestamp)
    , direction(direction)
{ }

bool LastStrikeInfo::operator<(const LastStrikeInfo &other) const
{
    if (!other.isValid())
        return true;

    if (!isValid())
        return false;

    if (distance < other.distance)
        return true;

    if (distance > other.distance)
        return false;

    return timestamp > other.timestamp;
}

#endif // LASTSTRIKEINFO_H
