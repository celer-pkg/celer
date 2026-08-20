// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

package main

import (
	context "context"
	"crypto/tls"
	"fmt"
	"io"
	"log"
	"net"
	"os"
	"qtgrpc/tests"
	"time"

	grpc "google.golang.org/grpc"
	grpcCodes "google.golang.org/grpc/codes"
	grpcCredentials "google.golang.org/grpc/credentials"
	grpcMetadata "google.golang.org/grpc/metadata"
	grpcStatus "google.golang.org/grpc/status"

	"google.golang.org/grpc/encoding"
	"google.golang.org/protobuf/encoding/protojson"
	"google.golang.org/protobuf/proto"
)

type JsonCodec struct {
}

func (JsonCodec) Name() string {
	return "json"
}

func (JsonCodec) Marshal(v interface{}) (out []byte, err error) {
	return protojson.Marshal(v.(proto.Message))
}

func (JsonCodec) Unmarshal(data []byte, v interface{}) (err error) {
	return protojson.Unmarshal(data, v.(proto.Message))
}

type testServer struct {
	testMessageLatency time.Duration
	tests.TestServiceServer
}

func (ts *testServer) TestMethod(ctx context.Context, req *tests.SimpleStringMessage) (*tests.SimpleStringMessage, error) {

	if req.TestFieldString == "sleep" {
		time.Sleep(ts.testMessageLatency)
	}
	return req, nil
}

func (ts *testServer) TestMethodServerStream(req *tests.SimpleStringMessage, serv tests.TestService_TestMethodServerStreamServer) error {
	for i := 1; i <= 4; i++ {
		resp := &tests.SimpleStringMessage{
			TestFieldString: req.TestFieldString + fmt.Sprintf("%d", i),
		}
		time.Sleep(ts.testMessageLatency)
		serv.Send(resp)
	}

	return nil
}

func (ts *testServer) TestMethodClientStream(stream tests.TestService_TestMethodClientStreamServer) error {
	var rspString string
	for i := 0; i < 4; i++ {
		req, err := stream.Recv()
		if err != nil {
			return grpcStatus.Error(grpcCodes.DataLoss, rspString)
		}
		rspString += req.TestFieldString + fmt.Sprintf("%d", i+1)
		time.Sleep(ts.testMessageLatency)
	}

	return stream.SendAndClose(&tests.SimpleStringMessage{TestFieldString: rspString})
}

func (ts *testServer) TestMethodBiStream(stream tests.TestService_TestMethodBiStreamServer) error {
	for i := 0; i < 4; i++ {
		req, err := stream.Recv()
		if err != nil {
			return grpcStatus.Error(grpcCodes.DataLoss, "Read failed")
		}

		rspString := req.TestFieldString + fmt.Sprintf("%d", i+1)
		err = stream.Send(&tests.SimpleStringMessage{TestFieldString: rspString})
		if err != nil {
			return grpcStatus.Error(grpcCodes.DataLoss, "Write failed")
		}
		time.Sleep(ts.testMessageLatency)
	}
	return nil
}

func (ts *testServer) TestMethodClientStreamWithDone(stream tests.TestService_TestMethodClientStreamWithDoneServer) error {
	var rspString string
	for i := 0; i < 4; i++ {
		req, err := stream.Recv()
		if err != nil {
			if err == io.EOF {
				return stream.SendAndClose(&tests.SimpleStringMessage{TestFieldString: rspString})
			} else {
				return grpcStatus.Error(grpcCodes.DataLoss, rspString)
			}
		}
		rspString += req.TestFieldString + fmt.Sprintf("%d", i+1)
		time.Sleep(ts.testMessageLatency)
	}

	return stream.SendAndClose(&tests.SimpleStringMessage{TestFieldString: rspString})
}

func (ts *testServer) TestMethodBiStreamWithDone(stream tests.TestService_TestMethodBiStreamWithDoneServer) error {
	for i := 0; i < 4; i++ {
		req, err := stream.Recv()
		if err != nil {
			if err == io.EOF {
				return nil
			} else {
				return grpcStatus.Error(grpcCodes.DataLoss, "Read failed")
			}
		}

		rspString := req.TestFieldString + fmt.Sprintf("%d", i+1)
		err = stream.Send(&tests.SimpleStringMessage{TestFieldString: rspString})
		if err != nil {
			return grpcStatus.Error(grpcCodes.DataLoss, "Write failed")
		}
		time.Sleep(ts.testMessageLatency)
	}
	return nil
}

func (ts *testServer) TestMethodBlobServerStream(req *tests.BlobMessage, serv tests.TestService_TestMethodBlobServerStreamServer) error {
	serv.Send(req)
	return nil
}

func (ts *testServer) TestMethodStatusMessage(ctx context.Context, req *tests.SimpleStringMessage) (*tests.SimpleStringMessage, error) {
	return nil, grpcStatus.Error(grpcCodes.Unimplemented, req.TestFieldString)
}

func (ts *testServer) TestMethodNonCompatibleArgRet(ctx context.Context, req *tests.SimpleIntMessage) (*tests.SimpleStringMessage, error) {
	return &tests.SimpleStringMessage{
		TestFieldString: fmt.Sprintf("%d", req.TestField),
	}, nil
}

func (ts *testServer) TestMetadata(ctx context.Context, req *tests.Empty) (*tests.Empty, error) {
	md, ok := grpcMetadata.FromIncomingContext(ctx)
	if !ok {
		return nil, grpcStatus.Error(grpcCodes.InvalidArgument, "Token invalid request")
	}

	clientHeader := md.Get("client_header")
	if len(clientHeader) != 1 {
		return nil, grpcStatus.Error(grpcCodes.Internal,
			"client_header is missing")
	}

	header := grpcMetadata.Pairs("server_header", clientHeader[0])
	grpc.SetHeader(ctx, header)

	clientHeaderRet := md.Get("client_return_header")
	if len(clientHeaderRet) != 1 {
		return nil, grpcStatus.Error(grpcCodes.Internal,
			"client_header is missing")
	}

	if clientHeaderRet[0] == "" {
		header = grpcMetadata.Pairs("client_return_header", "invalid_value")
	} else {
		header = grpcMetadata.Pairs("client_return_header", clientHeaderRet[0])
	}
	grpc.SetHeader(ctx, header)

	return &tests.Empty{}, nil
}

func main() {
	encoding.RegisterCodec(JsonCodec{})

	if len(os.Args) < 3 {
		log.Fatal("Message latency is missing")
	}
	if os.Args[1] != "--latency" {
		log.Fatalf("Not latency argument provided")
	}

	testMessageLatency, err := time.ParseDuration(os.Args[2] + "ms")
	if err != nil {
		log.Fatalf("Unable to parse message latency argument %s: %v", os.Args[1], err)
	}
	serv := &testServer{
		testMessageLatency: testMessageLatency,
	}
	lis, err := net.Listen("tcp", "localhost:50051")
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}

	testUnixSocket := "/tmp/qtgrpc_test.sock"
	os.Remove(testUnixSocket)
	lisUnix, err := net.Listen("unix", testUnixSocket)
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}

	var opts []grpc.ServerOption
	grpcServer := grpc.NewServer(opts...)
	tests.RegisterTestServiceServer(grpcServer, serv)
	go func() {
		grpcServer.Serve(lis)
	}()

	go func() {
		grpcServer.Serve(lisUnix)
	}()

	serverCert, err := tls.LoadX509KeyPair("cert.pem", "key.pem")
	if err != nil {
		log.Fatalf("Unable to load SSL certificate %v", err)
	}

	lisSsl, err := net.Listen("tcp", "localhost:50052")
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}

	grpcSslServer := grpc.NewServer(grpc.Creds(grpcCredentials.NewTLS(&tls.Config{
		Certificates: []tls.Certificate{serverCert},
		ClientAuth:   tls.NoClientCert,
	})))

	go func() {
		startTime, _ := time.ParseDuration("500ms")
		time.Sleep(startTime)
		fmt.Fprintln(os.Stderr, "Server listening")
	}()

	tests.RegisterTestServiceServer(grpcSslServer, serv)

	grpcSslServer.Serve(lisSsl)
}
