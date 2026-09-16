#pragma once

#include <string>
#include <vector>

#include "activities/Activity.h"
#include "util/ButtonNavigator.h"

class NotesActivity final : public Activity {
 private:
  ButtonNavigator buttonNavigator;
  std::vector<std::string> noteFiles;
  int selectorIndex = 0;
  bool newNoteRequested = false;

  void loadNotes();
  void createNote();
  void openSelected();
  static std::string displayName(const std::string& path);
  static std::string makeSafeFileName(const std::string& title);
  bool writeNote(const std::string& title, const std::string& body);

 public:
  explicit NotesActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Notes", renderer, mappedInput) {}

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;
};
