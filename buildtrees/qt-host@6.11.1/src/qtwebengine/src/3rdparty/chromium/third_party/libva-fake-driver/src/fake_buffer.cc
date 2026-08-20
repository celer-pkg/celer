// Copyright 2023 The Chromium Authors
// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fake_buffer.h"

namespace {

size_t CalculateDataSize(unsigned int size_per_element,
                         unsigned int num_elements) {
  // TODO(b/358445928): try to bring back safe math.
  return static_cast<size_t>(size_per_element) *
         static_cast<size_t>(num_elements);
}

}  // namespace

namespace libvafake {

FakeBuffer::FakeBuffer(IdType id,
                       VAContextID context,
                       VABufferType type,
                       unsigned int size_per_element,
                       unsigned int num_elements,
                       const void* data)
    : id_(id),
      context_(context),
      type_(type),
      data_size_(CalculateDataSize(size_per_element, num_elements)),
      data_(std::make_unique<uint8_t[]>(data_size_)) {
  if (data) {
    memcpy(data_.get(), data, data_size_);
  }
}

FakeBuffer::~FakeBuffer() = default;

FakeBuffer::IdType FakeBuffer::GetID() const {
  return id_;
}

VAContextID FakeBuffer::GetContextID() const {
  return context_;
}

VABufferType FakeBuffer::GetType() const {
  return type_;
}

size_t FakeBuffer::GetDataSize() const {
  return data_size_;
}

void* FakeBuffer::GetData() const {
  return data_.get();
}

}  // namespace libvafake
