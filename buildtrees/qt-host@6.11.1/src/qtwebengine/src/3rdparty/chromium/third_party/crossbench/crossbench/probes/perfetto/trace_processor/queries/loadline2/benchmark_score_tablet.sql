INCLUDE PERFETTO MODULE ext.loadline2_amazon_product;
INCLUDE PERFETTO MODULE ext.loadline2_cnn_article;
INCLUDE PERFETTO MODULE ext.loadline2_google_doc;
INCLUDE PERFETTO MODULE ext.loadline2_google_search_result;
INCLUDE PERFETTO MODULE ext.loadline2_youtube_video;

SELECT
  loadline2_amazon_product_score() as amazon_product,
  loadline2_cnn_article_score() as cnn_article,
  loadline2_google_doc_score() as google_doc,
  loadline2_google_search_result_score() as google_search_result,
  loadline2_youtube_video_score() as youtube_video;

