使用步骤:
1. 清空tts&sound目录下所有文件
2. 将需要转换成mp3的pcm文件拷贝到tts目录下
3. 将不需要转换的pcm文件拷贝到sound目录下
4. 执行脚本生成头文件:
	(1) run-mp3-high.bat（压缩低，音频质量高）
	(2) run-mp3-middle.bat（压缩中，音频质量中 - 建议）
	(3) run-mp3-low.bat（压缩高，音频质量低）
5. 拷贝output目录下tone.h&tone_buf.h到目的项目中使用