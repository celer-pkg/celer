INCLUDE PERFETTO MODULE ext.first_presentation_time;
INCLUDE PERFETTO MODULE ext.navigation_start;

-- This metric returns the time the cookie banner takes to disappear.
CREATE OR REPLACE PERFETTO FUNCTION loadline_youtube_video_score()
RETURNS FLOAT
AS
SELECT
  -- Multiply by 60 to make the score per minutes rather than per second.
  60e9
  / (get_first_presentation_time_for_event('cookie_banner_gone') - first_navigation_start());
