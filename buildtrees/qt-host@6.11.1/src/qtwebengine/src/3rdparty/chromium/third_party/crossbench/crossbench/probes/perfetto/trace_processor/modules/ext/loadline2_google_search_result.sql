INCLUDE PERFETTO MODULE ext.first_presentation_time;

CREATE OR REPLACE PERFETTO FUNCTION loadline2_google_search_result_score()
RETURNS FLOAT
AS
SELECT
  -- Multiply by 60 to make the score per minutes rather than per second.
  60e9 / (
    get_presentation_time('LoadLine2/google_search/result_added_to_dom')
    - get_event_time('LoadLine2/*/google_search_result_start'));

