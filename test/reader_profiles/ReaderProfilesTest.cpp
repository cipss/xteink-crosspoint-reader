#include <gtest/gtest.h>

#include "ReaderProfile.h"
#include "ReaderProfileStore.h"

TEST(ReaderProfiles, SerializeAndDeserializeRoundTrip) {
  ReaderProfile profile;
  profile.id = "PERSONALIZZATO";
  profile.isBuiltIn = false;
  profile.fontFamily = 0;
  profile.fontSize = 1;
  profile.lineSpacing = 2;
  profile.paragraphAlignment = 0;
  profile.extraParagraphSpacing = 1;
  profile.screenMargin = 10;
  profile.embeddedStyle = 1;
  profile.focusReadingEnabled = 0;
  profile.hyphenationEnabled = 1;
  profile.textAntiAliasing = 1;
  profile.imageRendering = 0;
  profile.orientation = 0;
  profile.refreshFrequency = 3;

  const auto json = profile.serializeToJsonString();
  auto roundTrip = ReaderProfile::deserializeFromJsonString(json);

  ASSERT_TRUE(roundTrip.has_value());
  EXPECT_EQ(roundTrip->id, profile.id);
  EXPECT_EQ(roundTrip->fontFamily, profile.fontFamily);
  EXPECT_EQ(roundTrip->fontSize, profile.fontSize);
  EXPECT_EQ(roundTrip->lineSpacing, profile.lineSpacing);
  EXPECT_EQ(roundTrip->paragraphAlignment, profile.paragraphAlignment);
  EXPECT_EQ(roundTrip->screenMargin, profile.screenMargin);
  EXPECT_EQ(roundTrip->embeddedStyle, profile.embeddedStyle);
  EXPECT_EQ(roundTrip->focusReadingEnabled, profile.focusReadingEnabled);
  EXPECT_EQ(roundTrip->hyphenationEnabled, profile.hyphenationEnabled);
  EXPECT_EQ(roundTrip->textAntiAliasing, profile.textAntiAliasing);
  EXPECT_EQ(roundTrip->imageRendering, profile.imageRendering);
  EXPECT_EQ(roundTrip->orientation, profile.orientation);
  EXPECT_EQ(roundTrip->refreshFrequency, profile.refreshFrequency);
}

TEST(ReaderProfiles, BuiltInMetadataIsStable) {
  const auto romanzo = ReaderProfile::createRomanzo();
  const auto studio = ReaderProfile::createStudio();
  const auto focus = ReaderProfile::createFocus();
  const auto pdf = ReaderProfile::createPdf();

  EXPECT_EQ(romanzo.id, "ROMANZO");
  EXPECT_EQ(studio.id, "STUDIO");
  EXPECT_EQ(focus.id, "FOCUS");
  EXPECT_EQ(pdf.id, "PDF");

  EXPECT_TRUE(romanzo.isBuiltIn);
  EXPECT_TRUE(studio.isBuiltIn);
  EXPECT_TRUE(focus.isBuiltIn);
  EXPECT_TRUE(pdf.isBuiltIn);
}
