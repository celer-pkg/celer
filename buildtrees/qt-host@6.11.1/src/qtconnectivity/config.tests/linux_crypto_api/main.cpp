// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: BSD-3-Clause

#include <linux/if_alg.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
    sockaddr_alg sa;
    sa.salg_family = AF_ALG;
    sa.salg_type[0] = 0;
    sa.salg_name[0] = 0;
    setsockopt(3, 279 /* SOL_ALG */, ALG_SET_KEY, "dummy", 5);
}
