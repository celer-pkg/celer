-- Copyright 2025 The Chromium Authors
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.
DROP TABLE IF EXISTS enum_table;

CREATE PERFETTO TABLE enum_table(
  histogram_name STRING,
  enum_name STRING,
  enum_value LONG)
AS
SELECT
  column1 AS histogram_name,
  column2 AS enum_name,
  column3 AS enum_value
FROM
  (
    VALUES('Viz.DisplayCompositor.OverlayStrategy', 'kUnknown', 0),
    ('Viz.DisplayCompositor.OverlayStrategy', 'kNoStrategyUsed', 1),
    ('Viz.DisplayCompositor.OverlayStrategy', 'kFullscreen', 2),
    ('Viz.DisplayCompositor.OverlayStrategy', 'kSingleOnTop', 3)
  );
