// Multithreaded CLI Media Player (Simulated Audio Playback)
// Basic C++ implementation using standard libraries

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <fstream>
#include <string>

class AudioTrack {
public:
    std::string fileName;
    int duration; // in seconds

    AudioTrack(const std::string& name, int dur)
        : fileName(name), duration(dur) {}

    void load() {
        std::cout << "[AudioTrack] Loaded track: " << fileName << " (" << duration << "s)\n";
    }
};

class MediaPlayer {
private:
    std::atomic<bool> isPlaying;
    std::atomic<bool> isStopped;
    std::thread playThread;
    std::mutex mtx;
    std::condition_variable cv;
    AudioTrack* currentTrack;
    int currentTime;

public:
    MediaPlayer() : isPlaying(false), isStopped(true), currentTrack(nullptr), currentTime(0) {}

    void loadTrack(AudioTrack& track) {
        currentTrack = &track;
        currentTrack->load();
        currentTime = 0;
    }

    void play() {
        if (!currentTrack) {
            std::cout << "[MediaPlayer] No track loaded.\n";
            return;
        }

        if (isPlaying) {
            std::cout << "[MediaPlayer] Already playing.\n";
            return;
        }

        isPlaying = true;
        isStopped = false;

        playThread = std::thread(&MediaPlayer::playbackLoop, this);
    }

    void pause() {
        std::unique_lock<std::mutex> lock(mtx);
        isPlaying = false;
        std::cout << "[MediaPlayer] Paused at " << currentTime << "s.\n";
    }

    void resume() {
        std::unique_lock<std::mutex> lock(mtx);
        isPlaying = true;
        cv.notify_one();
        std::cout << "[MediaPlayer] Resumed.\n";
    }

    void stop() {
        isPlaying = false;
        isStopped = true;
        cv.notify_one();
        if (playThread.joinable()) playThread.join();
        std::cout << "[MediaPlayer] Stopped.\n";
    }

    void playbackLoop() {
        std::ofstream logFile("playback.log", std::ios::app);
        while (!isStopped && currentTime < currentTrack->duration) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&]() { return isPlaying || isStopped; });

            if (isStopped) break;

            std::this_thread::sleep_for(std::chrono::seconds(1));
            currentTime++;
            std::cout << "[MediaPlayer] Playing: " << currentTrack->fileName << " - " << currentTime << "s\n";
            logFile << "Playing: " << currentTrack->fileName << " - " << currentTime << "s\n";
        }
        logFile.close();
        isPlaying = false;
    }
};

int main() {
    MediaPlayer player;
    AudioTrack track("sample.wav", 10);

    player.loadTrack(track);
    player.play();

    std::this_thread::sleep_for(std::chrono::seconds(3));
    player.pause();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    player.resume();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    player.stop();

    return 0;
}
