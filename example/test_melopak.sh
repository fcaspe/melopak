#Pack wave file in instances of 1204 samples.
mkdir -p out
../bin/melopak --len 1024 --rate 16000 --seq ./out.map --wave ./wav/test.wav --label ./out/labels.ubyte --pack ./out/audio.ubyte --ch M
rm out.map