#include "ActivityManager.h"

#include <FontCacheManager.h>
#include <HalPowerManager.h>

#include <algorithm>

#include "OpdsServerStore.h"
#include "boot_sleep/BootActivity.h"
#include "boot_sleep/SleepActivity.h"
#include "browser/OpdsBookBrowserActivity.h"
#include "home/CrashActivity.h"
#include "home/FileBrowserActivity.h"
#include "home/HomeActivity.h"
#include "home/RecentBooksActivity.h"
#include "network/CrossPointWebServerActivity.h"
#include "reader/ReaderActivity.h"
#include "settings/OpdsServerListActivity.h"
#include "settings/SettingsActivity.h"
#include "util/FullScreenMessageActivity.h"
#include "x3plus/X3PlusLauncherActivity.h"
#include "x3plus/manga/MangaLibraryActivity.h"
#include "x3plus/manga/MangaReaderActivity.h"

void ActivityManager::begin() {
  xTaskCreate(&renderTaskTrampoline, "ActivityManagerRender",
              8192,
              this,
              1,
              &renderTaskHandle);
  assert(renderTaskHandle != nullptr && "Failed to create render task");
}

void ActivityManager::renderTaskTrampoline(void* param) {
  auto* self = static_cast<ActivityManager*>(param);
  self->renderTaskLoop();
}

void ActivityManager::renderTaskLoop() {
  while (true) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    RenderLock lock;
    if (currentActivity) {
      HalPowerManager::Lock powerLock;
      currentActivity->render(std::move(lock));
    }
    TaskHandle_t waiter = nullptr;
    taskENTER_CRITICAL(nullptr);
    waiter = waitingTaskHandle;
    waitingTaskHandle = nullptr;
    taskEXIT_CRITICAL(nullptr);
    if (waiter) xTaskNotify(waiter, 1, eIncrement);
  }
}

void ActivityManager::loop() {
  if (currentActivity) currentActivity->loop();

  while (pendingAction != PendingAction::None) {
    if (pendingAction == PendingAction::Pop) {
      RenderLock lock;
      if (!currentActivity) {
        LOG_ERR("ACT", "Pop set but currentActivity is null; ignoring pop request");
        pendingAction = PendingAction::None;
        continue;
      }
      ActivityResult pendingResult = std::move(currentActivity->result);
      exitActivity(lock);
      pendingAction = PendingAction::None;
      if (stackActivities.empty()) {
        lock.unlock();
        goHome();
        continue;
      }
      currentActivity = std::move(stackActivities.back());
      stackActivities.pop_back();
      if (currentActivity->resultHandler) {
        auto handler = std::move(currentActivity->resultHandler);
        currentActivity->resultHandler = nullptr;
        lock.unlock();
        handler(pendingResult);
      }
      if (pendingAction == PendingAction::None) requestUpdate();
      continue;
    }

    if (pendingActivity) {
      RenderLock lock;
      if (pendingAction == PendingAction::Replace) {
        exitActivity(lock);
        while (!stackActivities.empty()) {
          stackActivities.back()->onExit();
          stackActivities.pop_back();
        }
      } else if (pendingAction == PendingAction::Push) {
        stackActivities.push_back(std::move(currentActivity));
      }
      pendingAction = PendingAction::None;
      currentActivity = std::move(pendingActivity);
      lock.unlock();
      currentActivity->onEnter();
      continue;
    }
  }

  if (requestedUpdate) {
    requestedUpdate = false;
    if (renderTaskHandle) xTaskNotify(renderTaskHandle, 1, eIncrement);
  }
}

void ActivityManager::exitActivity(const RenderLock&) {
  if (currentActivity) {
    currentActivity->onExit();
    currentActivity.reset();
  }
}

void ActivityManager::replaceActivity(std::unique_ptr<Activity>&& newActivity) {
  if (currentActivity) {
    pendingActivity = std::move(newActivity);
    pendingAction = PendingAction::Replace;
  } else {
    currentActivity = std::move(newActivity);
    currentActivity->onEnter();
  }
}

void ActivityManager::goToFileTransfer() {
  replaceActivity(std::make_unique<CrossPointWebServerActivity>(renderer, mappedInput));
}

void ActivityManager::goToX3Plus() {
  replaceActivity(std::make_unique<X3PlusLauncherActivity>(renderer, mappedInput));
}

void ActivityManager::goToMangaLibrary() {
  replaceActivity(std::make_unique<MangaLibraryActivity>(renderer, mappedInput));
}

void ActivityManager::goToMangaReader(std::string path) {
  replaceActivity(std::make_unique<MangaReaderActivity>(renderer, mappedInput, std::move(path)));
}

void ActivityManager::goToSettings() { replaceActivity(std::make_unique<SettingsActivity>(renderer, mappedInput)); }

void ActivityManager::goToFileBrowser(std::string path) {
  replaceActivity(std::make_unique<FileBrowserActivity>(renderer, mappedInput, std::move(path)));
}

void ActivityManager::goToRecentBooks() {
  replaceActivity(std::make_unique<RecentBooksActivity>(renderer, mappedInput));
}

void ActivityManager::goToBrowser() {
  const auto& servers = OPDS_STORE.getServers();
  if (servers.size() == 1) {
    replaceActivity(std::make_unique<OpdsBookBrowserActivity>(renderer, mappedInput, servers[0]));
  } else {
    replaceActivity(std::make_unique<OpdsServerListActivity>(renderer, mappedInput, true));
  }
}

void ActivityManager::goToReader(std::string path) {
  replaceActivity(std::make_unique<ReaderActivity>(renderer, mappedInput, std::move(path)));
}

void ActivityManager::goToSleep(bool fromTimeout) {
  replaceActivity(std::make_unique<SleepActivity>(renderer, mappedInput, fromTimeout));
  loop();
}

void ActivityManager::goToBoot() { replaceActivity(std::make_unique<BootActivity>(renderer, mappedInput)); }

void ActivityManager::goToFullScreenMessage(std::string message, EpdFontFamily::Style style) {
  replaceActivity(std::make_unique<FullScreenMessageActivity>(renderer, mappedInput, std::move(message), style));
}

void ActivityManager::goHome(HomeMenuItem initialMenuItem) {
  if (initialMenuItem == HomeMenuItem::NONE && currentActivity) {
    const auto& activityName = currentActivity->name;
    if (activityName == "FileBrowser") {
      initialMenuItem = HomeMenuItem::FILE_BROWSER;
    } else if (activityName == "RecentBooks") {
      initialMenuItem = HomeMenuItem::RECENTS;
    } else if (activityName == "OpdsBookBrowser") {
      initialMenuItem = HomeMenuItem::OPDS_BROWSER;
    } else if (activityName == "CrossPointWebServer") {
      initialMenuItem = HomeMenuItem::FILE_TRANSFER;
    } else if (activityName == "Settings") {
      initialMenuItem = HomeMenuItem::SETTINGS_MENU;
    } else if (activityName == "X3PlusLauncher") {
      initialMenuItem = HomeMenuItem::X3PLUS;
    }
  }
  replaceActivity(std::make_unique<HomeActivity>(renderer, mappedInput, initialMenuItem));
}

void ActivityManager::goToCrashReport() { replaceActivity(std::make_unique<CrashActivity>(renderer, mappedInput)); }

void ActivityManager::pushActivity(std::unique_ptr<Activity>&& activity) {
  if (pendingActivity) {
    LOG_ERR("ACT", "pendingActivity while pushActivity is not expected");
    pendingActivity.reset();
  }
  pendingActivity = std::move(activity);
  pendingAction = PendingAction::Push;
}

void ActivityManager::popActivity() {
  if (pendingActivity) {
    LOG_ERR("ACT", "pendingActivity while popActivity is not expected");
    pendingActivity.reset();
  }
  pendingAction = PendingAction::Pop;
}

bool ActivityManager::preventAutoSleep() const { return currentActivity && currentActivity->preventAutoSleep(); }

bool ActivityManager::isReaderActivity() const {
  return std::any_of(stackActivities.begin(), stackActivities.end(),
                     [](const auto& activity) { return activity->isReaderActivity(); }) ||
         (currentActivity && currentActivity->isReaderActivity());
}

bool ActivityManager::skipLoopDelay() const { return currentActivity && currentActivity->skipLoopDelay(); }

ScreenshotInfo ActivityManager::getScreenshotInfo() const {
  if (currentActivity) return currentActivity->getScreenshotInfo();
  return {};
}

void ActivityManager::requestUpdate(bool immediate) {
  if (immediate) {
    if (renderTaskHandle) xTaskNotify(renderTaskHandle, 1, eIncrement);
  } else {
    requestedUpdate = true;
  }
}

void ActivityManager::requestUpdateAndWait() {
  if (!renderTaskHandle) return;
  taskENTER_CRITICAL(nullptr);
  auto currTaskHandler = xTaskGetCurrentTaskHandle();
  auto mutexHolder = xSemaphoreGetMutexHolder(renderingMutex);
  bool isRenderTask = (currTaskHandler == renderTaskHandle);
  bool alreadyWaiting = (waitingTaskHandle != nullptr);
  bool holdingRenderLock = (mutexHolder == currTaskHandler);
  if (!alreadyWaiting && !isRenderTask && !holdingRenderLock) waitingTaskHandle = currTaskHandler;
  taskEXIT_CRITICAL(nullptr);
  assert(!isRenderTask && "Render task cannot call requestUpdateAndWait()");
  assert(!alreadyWaiting && "Already waiting for a render to complete");
  assert(!holdingRenderLock && "Cannot call requestUpdateAndWait() while holding RenderLock");
  xTaskNotify(renderTaskHandle, 1, eIncrement);
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

RenderLock::RenderLock() {
  xSemaphoreTake(activityManager.renderingMutex, portMAX_DELAY);
  isLocked = true;
}

RenderLock::RenderLock([[maybe_unused]] Activity&) {
  xSemaphoreTake(activityManager.renderingMutex, portMAX_DELAY);
  isLocked = true;
}

RenderLock::~RenderLock() {
  if (isLocked) {
    xSemaphoreGive(activityManager.renderingMutex);
    isLocked = false;
  }
}

void RenderLock::unlock() {
  if (isLocked) {
    xSemaphoreGive(activityManager.renderingMutex);
    isLocked = false;
  }
}

bool RenderLock::peek() { return xQueuePeek(activityManager.renderingMutex, NULL, 0) != pdTRUE; }
