// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "exampleservice.grpc.pb.h"

#include <grpc++/grpc++.h>

#include <array>
#include <iostream>

using namespace qtgrpc::examples;

// Logic and data behind the server's behavior.
class ExampleServiceImpl final : public ExampleService::Service
{
    inline static std::array<std::string, 10> answers = {
        "Yes",  "Yep",       "Most\nlikely",   "It is\ncertain", "No",
        "Nope", "Try later", "Are you\nsure?", "Maybe",          "Very\ndoubtful"
    };

    std::string getRandomAnswer()
    {
        return answers.at(rand() % answers.size());
    }
//! [answerMethod]
    grpc::Status answerMethod(grpc::ServerContext *, const AnswerRequest *request,
                              AnswerResponse *response) override
    {
        if (request->question().empty()) {
            std::cerr << "Question is empty" << std::endl;
            return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Question is empty");
        }
        std::cout << "Received question: " << request->question() << std::endl;

        response->set_message(getRandomAnswer());

        return grpc::Status();
    };
//! [answerMethod]
};

int main(int argc, char *argv[])
{
    ExampleServiceImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort("127.0.0.1:50051", grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    if (!server) {
        std::cout << "Creating server failed." << std::endl;
        return -1;
    }

    std::cout << "Server listening on port 50051" << std::endl;
    server->Wait();

    return 0;
}
