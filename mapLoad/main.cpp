// 目標1、ファイルを読み込んで四角単位のステージ生成
#include "GameLib\Framework.h"
using namespace GameLib;

#include <fstream>
#include <algorithm>
using namespace std;

void readFile(const char *fName, char **buffer, int *fSize);
void mainLoop();

// 二次元配列クラス
template< class T > class Array2D {
public:
	Array2D() : mArray(0) {}
	~Array2D() {
		delete[] mArray;
		mArray = NULL;
	}
	void setSize(int size0, int size1) {
		mSize0 = size0;
		mSize1 = size1;
		mArray = new T[size0 * size1];
	}
	T& operator()(int index0, int index1) {
		return mArray[index1 * mSize0 + index0];
	}
	const T& operator()(int index0, int index1) const {
		return mArray[index1 * mSize0 + index0];
	}
private:
	T* mArray;
	int mSize0;
	int mSize1;
};

// 状態クラス
class State {
public:
	State(const char* stageDate, int size);
	void update(char input);
	void draw() const;
	bool hasCleared() const;
private:
	enum Object {
		OBJ_SPACE,
		OBJ_WALL,
		OBJ_BLOCK,
		OBJ_MAN,

		OBJ_UNKNOWN
	};
	void setSize(const char* stageData, int size);
	void drawCell(int x, int y, unsigned color) const;

	int mWidth;
	int mHeight;
	Array2D< Object > mObjects;
	Array2D< bool > mGoalFlags;
};

namespace GameLib {
	void Framework::update() {
		mainLoop();
	}
}

State *gState = NULL;

void mainLoop() {
	if (!gState) {
		const char *fName = "map2.txt";
		char *mapData = NULL;
		int fSize;
		readFile(fName, &mapData, &fSize);

		if (!mapData) {
			cout << "map file could not be read." << endl;
			return;
		}
	
		gState = new State(mapData, fSize);
		//State st(mapData, fSize);
		delete[] mapData;
		mapData = NULL;
		// 初回描画
		gState->draw();
		return;
		//st.draw();

	}

	char input;
	cin >> input;
	gState->update(input);
	gState->draw();


	//gState = new State(mapData, fSize);
	//gState->draw();
}


void readFile(const char *fName, char **buffer, int *fSize) {
	ifstream in(fName, ios::binary);
	if (!in) {
		return;
	}
	in.seekg(0, ios::end);
	*fSize = static_cast<int>(in.tellg());
	in.seekg(0, ios::beg);
	*buffer = new char[*fSize];
	in.read(*buffer, *fSize);
}

State::State(const char* stageData, int size) {
	// 改行文字を除いたステージの幅、高さを設定
	setSize(stageData, size);
	// setSize()で求めた幅、高さを元に配列を確保
	mObjects.setSize(mWidth, mHeight);
	mGoalFlags.setSize(mWidth, mHeight);
	// サンプルでは初期化の処理を書いてあるが、あえて書かない。実験
	// エラー起きたから書いてみる
	for (int y = 0; y < mHeight; ++y) {
		for (int x = 0; x < mWidth; ++x) {
			mObjects(x, y) = OBJ_WALL;
			mGoalFlags(x, y) = false;
		}
	}

	// Array2D <Object>& o = mObjects; これいらなかった
	Object t;
	bool goalFlag = false;
	int x = 0;
	int y = 0;


	for (int i = 0; i < size; i++) {
		switch (stageData[i])
		{
		case '#': t = OBJ_WALL; break;
		case '.': t = OBJ_SPACE; break; goalFlag = true; // SPACEで良いの？謎
		case 'p': t = OBJ_MAN;  break;
		case 'o': t = OBJ_BLOCK; break;
		case ' ': t = OBJ_SPACE; break;
		case '\n': y++; x = 0; t = OBJ_UNKNOWN; break;
		default: t = OBJ_UNKNOWN; break;
		}
		if (t != OBJ_UNKNOWN) {
			mObjects(x, y) = t;
			mGoalFlags(x, y) = goalFlag;
			// 値が入る度に幅は広がるから、xをインクリメント
			++x;
		}
	}

}

void State::setSize(const char* stageData, int size) {
	mWidth = mHeight = 0;
	int x = 0; int y = 0;

	for (int i = 0; i < size; i++) {
		switch (stageData[i])
		{
		case '#': x++; break;
		case '.': x++; break;
		case 'p': x++; break;
		case 'o': x++; break;
		case ' ': x++; break;
		case '\n':
			y++;
			// 幅、高さの最大値を更新
			mWidth = max(mWidth, x);
			mHeight = max(mHeight, y);
			x = 0;
			break;
		default:
			break;
		}

	}
}

void State::drawCell(int x, int y, unsigned color)  const{
	unsigned *vram = Framework::instance().videoMemory();
	int width = Framework::instance().width();

	for (int i = (y * 16); i < (y * 16 + 16); ++i) {
		for (int j = (x * 16); j < (x * 16 + 16); ++j) {
			vram[i * width + j] = color;
		}
	}
}

void State::draw() const {

	for (int y = 0; y < mHeight; y++) {
		for (int x = 0; x < mWidth; x++) {
			Object o = mObjects(x, y);
			bool g = mGoalFlags(x, y);
			unsigned color = 0;
			switch (o)
			{
			case OBJ_WALL: color = 0xffffff; break; // 壁は白
			case OBJ_SPACE: 
				if (g == true) {
					color = 0x00ffff; // ゴールは緑
					break;
				}
				else {
					color = 0x000000; // スペースは黒
					break;
				}
			case OBJ_MAN: color = 0xff0000; break; // プレイヤーはは赤
			case OBJ_BLOCK: color = 0x0000ff; break; // ブロックは青
				// case OBJ_UNKNOWN: y++; x = 0; break; これはもはや存在しない
			}
				drawCell(x, y, color);
		}
	}
}

void State::update(char input) {
	// 差を求める
	int dx = 0; int dy = 0;
	switch (input)
	{
	case 'a': dx = -1; break; // left
	case 's': dx = 1; break; // right
	case 'w': dy = -1; break; // up
	case 'z': dy = 1; break; // down
	default: return;
	}

	int x, y;
	bool f = false;
	for (y = 0; y < mHeight; y++) {
		for (x = 0; x < mWidth; x++) {
			Object o = mObjects(x, y);
			if (o == OBJ_MAN) {
				f = true;
				break;
			}
		}
		if (f == true) {
			break;
		}
	}
	
	int tx = x + dx;
	int ty = y + dy;
	if (tx < 0 || ty < 0 || tx >= mWidth || ty >= mHeight) {
		return;
	}
	
	if (mObjects(tx, ty) == OBJ_SPACE) {
		mObjects(x, y) = OBJ_SPACE;
		mObjects(tx, ty) = OBJ_MAN;
	}
	else if (mObjects(tx, ty) == OBJ_BLOCK) {
		int tx2 = tx + dx;
		int ty2 = ty + dy;

		if (tx2 < 0 || ty2 < 0 || tx2 >= mWidth || ty2 >= mHeight) {
			return;
		}
		if (mObjects(tx + dx, ty + dy) == OBJ_SPACE) {
			mObjects(tx + dx, ty + dy) = OBJ_BLOCK;
			mObjects(tx, ty) = OBJ_MAN;
			mObjects(x, y) = OBJ_SPACE;

		}
	}
}
