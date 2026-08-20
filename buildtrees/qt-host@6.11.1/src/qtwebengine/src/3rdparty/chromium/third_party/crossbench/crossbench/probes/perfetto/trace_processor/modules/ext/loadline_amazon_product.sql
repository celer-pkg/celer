INCLUDE PERFETTO MODULE ext.first_presentation_time;
INCLUDE PERFETTO MODULE ext.navigation_start;

-- This metric returns the time it takes for the "main" JS script to finish the
-- execution - this is when the page becomes interactive.
CREATE OR REPLACE PERFETTO FUNCTION loadline_amazon_product_score()
RETURNS FLOAT
AS
WITH
  js_ready AS (
    SELECT MAX(ts) AS js_ready
    FROM slice
    WHERE
      name = 'v8.run'
      AND EXTRACT_ARG(arg_set_id, 'debug.fileName')
        = 'https://www.amazon.co.uk/NIVEA-Suncream-Spray-Protect-Moisture/dp/B001B0OJXM'
  )
SELECT
  -- Multiply by 60 to make the score per minutes rather than per second.
  60e9 / (
    get_next_presentation_time(
      (SELECT * FROM js_ready))
    - first_navigation_start());
