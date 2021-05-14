#include <gst/gst.h>
#include <gtest/gtest.h>

TEST(basic_test, test_init)
{
  GstElement * fixator =
      gst_element_factory_make ("videoformatfixator", nullptr);

  ASSERT_NE (fixator, nullptr);
  gst_object_unref (fixator);
}

int
main(int argc, char **argv)
{
  int ret;

  ::testing::InitGoogleTest(&argc, argv);
  gst_init (nullptr, nullptr);

  ret = RUN_ALL_TESTS();
  gst_deinit ();

  return ret;
}