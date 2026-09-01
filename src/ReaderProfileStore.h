#pragma once

#include <string>
#include <vector>

#include "ReaderProfile.h"

class CrossPointSettings;

class ReaderProfileStore {
 private:
  static ReaderProfileStore instance;

  std::vector<ReaderProfile> profiles;
  std::string activeProfileId = "PERSONALIZZATO";

  // Private default constructor for singleton
  ReaderProfileStore() = default;

 public:
  ReaderProfileStore(const ReaderProfileStore&) = delete;
  ReaderProfileStore& operator=(const ReaderProfileStore&) = delete;

  static ReaderProfileStore& getInstance() { return instance; }

  bool saveToFile() const;
  bool loadFromFile();

  bool ensureDefaultProfiles(const CrossPointSettings& settings);
  bool setActiveProfile(const std::string& id);
  const ReaderProfile* getActiveProfile() const;
  const ReaderProfile* findProfileById(const std::string& id) const;
  const std::vector<ReaderProfile>& getProfiles() const { return profiles; }
  bool setProfiles(const std::vector<ReaderProfile>& nextProfiles) {
    profiles = nextProfiles;
    return !profiles.empty();
  }
  const std::string& getActiveProfileId() const { return activeProfileId; }

  void clear();
};

#define READER_PROFILE_STORE ReaderProfileStore::getInstance()
