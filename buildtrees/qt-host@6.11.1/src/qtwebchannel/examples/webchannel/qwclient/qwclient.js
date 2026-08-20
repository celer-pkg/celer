#!/usr/bin/env node
// Copyright (C) 2016 basysKom GmbH, author Sumedha Widyadharma <sumedha.widyadharma@basyskom.com>
// Copyright (C) 2016 basysKom GmbH, author Lutz Schönemann <lutz.schoenemann@basyskom.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

'use strict';
var repl = require('repl');
var WebSocket = require('faye-websocket').Client;
var QWebChannel = new require('./qwebchannel.js').QWebChannel;

var serverAddress = 'ws://localhost:12345';
var channels = [];

var autoConnect = process.argv.pop();
if (autoConnect === __filename) {
    autoConnect = false;
}

var openChannel = function(address) {
    // this should be bound to the repl
    var self = this;
    address = address ? address : serverAddress;
    if (address.indexOf('://') === -1) {
        address = 'ws://' + address;
    }

    var ws = new WebSocket(address);

    ws.on('open', function(event) {
        var transport = {
            onmessage: function(data) {},
            send: function(data) {
                ws.send(data, {
                    binary: false
                });
            }
        };
        ws.on('message', function(event) {
            transport.onmessage(event);
        }); // onmessage

        var webChannel = new QWebChannel(transport, function(channel) {
            channels.push(channel);
            var channelIdx = (channels.length - 1);
            console.log('channel opened', channelIdx);
            // Create a nice alias to access this channels objects
            self.context['c' + channelIdx] = channel.objects;

            ws.on('close', function() {
                for (var i = 0; i < channels.length; ++i) {
                    if (channels[i] === channel) {
                        console.log('channel closed', i);
                        channels[i] = null;
                        return;
                    }
                }
            }); // onclose
        }); // new QWebChannel
    }); // onopen

    ws.on('error', function(error) {
        console.log('websocket error', error.message);
    });
}; // openChannel

var setupRepl = function() {
    var r = repl.start({
        prompt: "webchannel> ",
        input: process.stdin,
        output: process.stdout,
        ignoreUndefined: true
    });

    r.context.serverAddress = serverAddress;
    r.context.openChannel = openChannel.bind(r);
    r.context.channels = channels;

    r.context.lsObjects = function() {
        for (let i = 0; i < channels.length; ++i) {
            const channel = channels[i];
            if (!channel) // closed and removed channel in repl
                continue;

            console.log('-- Channel "c' + i + '" objects:');
            for (const obj of Object.keys(channel.objects))
                console.log(obj, ':', channel.objects[obj]);
        }
    }
    return r;
}

var welcome = function() {
    console.log('Welcome to the qwebchannel/websocket REPL.');
    console.log('Use openChannel(url) to connect to a service.');
    console.log('For the standalone example, just openChannel() should suffice.');
    console.log('Opened channels have their objects aliased to c<channel number>, i.e. c0');
    console.log('So for the standalone example try: c0.core.receiveText(\'hello world\')');
}

welcome();
var repl = setupRepl();

if (autoConnect) {
    repl.context.openChannel(autoConnect);
}
