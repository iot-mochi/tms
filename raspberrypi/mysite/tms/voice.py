#coding: utf-8
import subprocess
import tempfile
import wave
import os

fname = 'voice.wav'
CHUNK = 512

def play(message, voice_path=fname):
    speed = 0.5
    dic_path = "/var/lib/mecab/dic/open-jtalk/naist-jdic"
    model_path = "/usr/share/hts-voice/mei/mei_normal.htsvoice"

    with tempfile.NamedTemporaryFile(mode='w+') as tmp:
        tmp.write(message)
        tmp.seek(0)
        command = 'open_jtalk -x {} -m {} -r {} -ow {} {}'.format(dic_path, model_path, speed, voice_path, tmp.name)
        print(command)
        proc = subprocess.run(
            command,
            shell  = True,
        )
    os.system("aplay --quiet '" + fname + "'")

if __name__ == '__main__':
    play("ゆ")
