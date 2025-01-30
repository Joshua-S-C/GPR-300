/*
*	Author: Eric Winebrenner
*/

#pragma once

namespace ew {
	unsigned int loadTexture(const char* filePath);
	unsigned int loadTexture(const char* filePath, int wrapMode, int magFilter, int minFilter, bool mipmap);
}
