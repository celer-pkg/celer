// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/data_decoder/public/cpp/json_sanitizer.h"

#include <utility>

#include "base/functional/bind.h"
#include "base/json/json_writer.h"
#include "base/types/expected.h"
#include "base/values.h"
#include "services/data_decoder/public/cpp/data_decoder.h"

namespace data_decoder {

// static
void JsonSanitizer::Sanitize(const std::string& json, Callback callback) {
  DataDecoder::ParseJsonIsolated(
      json,
      base::BindOnce(
          [](Callback callback, DataDecoder::ValueOrError parse_result) {
#if defined(_MSC_VER) && _MSC_VER < 1930
            if (!parse_result.has_value()) {
              std::move(callback).Run(base::unexpected(parse_result.error()));
              return;
            }
            const base::Value::Type type = parse_result->type();
            if (type != base::Value::Type::DICT &&
                type != base::Value::Type::LIST) {
              std::move(callback).Run(
                  base::unexpected("Invalid top-level type"));
              return;
            }
            std::string safe_json;
            if (!base::JSONWriter::Write(*parse_result, &safe_json)) {
              std::move(callback).Run(base::unexpected("Encoding error"));
              return;
            }
            std::move(callback).Run(base::ok(std::move(safe_json)));
#else

                  std::move(callback).Run(std::move(parse_result).and_then(
                      [](const base::Value& value) -> JsonSanitizer::Result {
                        if (value.type() != base::Value::Type::DICT &&
                            value.type() != base::Value::Type::LIST) {
                          return base::unexpected("Invalid top-level type");
                        }
                        std::string safe_json;
                        if (!base::JSONWriter::Write(value, &safe_json)) {
                          return base::unexpected("Encoding error");
                        }
                        return base::ok(safe_json);
                      }));
#endif
          },
          std::move(callback)));
}

}  // namespace data_decoder
