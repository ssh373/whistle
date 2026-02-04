#include <soundcapturing.h>

int main(int /* argc */, char** /* argv */) {
    SoundCapturing sound_capturing;
    // sound_capturing.process("default");
    // sound_capturing.process("hw:1,0");
    sound_capturing.process("plughw:1,0");
}
