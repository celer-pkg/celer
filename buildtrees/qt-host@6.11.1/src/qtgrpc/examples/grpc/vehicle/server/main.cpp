// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "navigationservice.grpc.pb.h"
#include "vehicleservice.grpc.pb.h"

#include <grpc++/grpc++.h>

#include <array>
#include <chrono>
#include <iostream>
#include <thread>

using namespace qtgrpc::examples;

class VehicleServiceImpl final : public VehicleService::Service
{
    constexpr static auto speedStreamPeriod = std::chrono::milliseconds(250);
    constexpr static auto rpmStreamPeriod = std::chrono::milliseconds(400);

    int speed = 0;
    int rpm = 900;
    int autonomy = 162;

    grpc::Status getSpeedStream(grpc::ServerContext *, const ::google::protobuf::Empty *,
                                grpc::ServerWriter<SpeedMsg> *writer) override
    {
        SpeedMsg msg;
        bool speedRequired = true;
        bool speedIncreasing = true;

        while (speedRequired) {
            if (speed > 180) {
                speedIncreasing = false;
            } else if (speed < 60) {
                speedIncreasing = true;
            }

            int deltaSpeed = rand() % 6 + 1; // Random number between 1 and 6
            if (speedIncreasing) {
                speed += deltaSpeed;
            } else {
                speed -= deltaSpeed;
            }

            msg.set_speed(speed);
            std::this_thread::sleep_for(speedStreamPeriod);
            speedRequired = writer->Write(msg);
        }
        return grpc::Status();
    }

    grpc::Status getRpmStream(grpc::ServerContext *, const ::google::protobuf::Empty *,
                              grpc::ServerWriter<RpmMsg> *writer) override
    {
        RpmMsg msg;
        bool rpmRequired = true;
        bool rpmIncreasing = true;
        while (rpmRequired) {
            if (rpm > 4800) {
                rpmIncreasing = false;
            } else if (rpm < 2200) {
                rpmIncreasing = true;
            }

            if (rpmIncreasing) {
                rpm += 100;
            } else {
                rpm -= 100;
            }

            msg.set_rpm(rpm);
            std::this_thread::sleep_for(rpmStreamPeriod);
            rpmRequired = writer->Write(msg);
        }

        return grpc::Status();
    }

    grpc::Status getAutonomy(grpc::ServerContext *, const ::google::protobuf::Empty *,
                             AutonomyMsg *response) override
    {
        response->set_autonomy(autonomy);
        if (autonomy > 10) {
            autonomy -= 1;
        }
        return grpc::Status();
    }
};

class NavigationServiceImpl final : public NavigationService::Service
{
    constexpr static std::chrono::seconds navigationPeriod = std::chrono::seconds(1);

    int totaldistance = 1200;
    inline static std::array<std::string, 3> streets = { "Friedrichstraße", "Kurfürstendamm",
                                                         "Erich-Thilo Straße" };

    grpc::Status getNavigationStream(grpc::ServerContext *, const ::google::protobuf::Empty *,
                                     grpc::ServerWriter<NavigationMsg> *writer) override
    {
        volatile bool navigationRequired = true;
        NavigationMsg msg;
        msg.set_totaldistance(totaldistance);

        for (auto street : streets) {
            msg.set_street(street);

            int traveledDistance = 0;
            msg.set_traveleddistance(traveledDistance);

            while (navigationRequired) {
                if (traveledDistance < totaldistance) {
                    traveledDistance += 50;
                } else {
                    traveledDistance = 0;
                }
                msg.set_traveleddistance(traveledDistance);

                if (traveledDistance <= 300) {
                    msg.set_direction(qtgrpc::examples::DirectionEnum::STRAIGHT);
                } else if (traveledDistance <= 600) {
                    msg.set_direction(qtgrpc::examples::DirectionEnum::RIGHT);
                } else if (traveledDistance <= 900) {
                    msg.set_direction(qtgrpc::examples::DirectionEnum::LEFT);
                }

                std::this_thread::sleep_for(navigationPeriod);
                navigationRequired = writer->Write(msg);
            }
        }

        return grpc::Status();
    }
};

int main()
{
    VehicleServiceImpl vehicleService;
    NavigationServiceImpl navigationService;

    grpc::ServerBuilder builder;
    builder.AddListeningPort("127.0.0.1:50051", grpc::InsecureServerCredentials());
    builder.RegisterService(&vehicleService);
    builder.RegisterService(&navigationService);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    if (!server) {
        std::cerr << "Creating grpc::Server failed.";
        return -1;
    }

    std::cout << "Server listening on port 50051";
    server->Wait();

    return 0;
}
