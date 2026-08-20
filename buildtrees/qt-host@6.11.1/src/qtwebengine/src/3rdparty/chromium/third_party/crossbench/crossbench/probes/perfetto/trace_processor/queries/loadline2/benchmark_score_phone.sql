INCLUDE PERFETTO MODULE ext.loadline2_amazon_product;
INCLUDE PERFETTO MODULE ext.loadline2_cnn_article;
INCLUDE PERFETTO MODULE ext.loadline2_globo_homepage;
INCLUDE PERFETTO MODULE ext.loadline2_google_search_result;
INCLUDE PERFETTO MODULE ext.loadline2_wikipedia_article;

SELECT
  loadline2_amazon_product_score() as amazon_product,
  loadline2_cnn_article_score() as cnn_article,
  loadline2_wikipedia_article_score() as wikipedia_article,
  loadline2_globo_homepage_score() as globo_homepage,
  loadline2_google_search_result_score() as google_search_result;

