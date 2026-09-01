# FMT_PARAM=-r -s 16000 -m m --bitwidth 16
export LD_LIBRARY_PATH=./libs:$LD_LIBRARY_PATH
rm output -rf
mkdir output/
mkdir output/amp
mkdir output/ring

chmod +x ./ToneTool/ToneTool
chmod +x ./ToneTool/lame
chmod +x ./ToneTool/tone_tool_header

./ToneTool/ToneTool tts/ 200 200


for file in output/amp/*; do
    if [ -f "$file" ]; then
        # echo "$file"
        path_not_suffix=${file%.*}
        # echo $path_not_suffix
        # path=$(dirname -- "$file")
        name=$(basename -- "$path_not_suffix")
        # echo "$path"
        # echo "$name"
        mp3_path=./output/ring/"$name".mp3
        echo "$mp3_path"
        ./ToneTool/lame -V0 -b 8 -B 8 -T -r -s 16000 -m m --bitwidth 16 $file "$mp3_path"
    fi
done

cp sound/* output/ring/

rm output/amp -rf

cd output

../ToneTool/tone_tool_header ring/