// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: BSD-3-Clause
#include <linux/can.h>
#include <linux/can/raw.h>

int main()
{
    canfd_frame frame;
    int fd_payload = CANFD_MAX_DLEN;
    fd_payload = CAN_RAW_FD_FRAMES;
    fd_payload = CANFD_MTU;
    return 0;
}

