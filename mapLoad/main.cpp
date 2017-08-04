// �ڕW1�A�t�@�C����ǂݍ���Ŏl�p�P�ʂ̃X�e�[�W����
#include "GameLib\Framework.h"
using namespace GameLib;

#include <fstream>
#include <algorithm>
using namespace std;

void readFile(const char *fName, char **buffer, int *fSize);
void mainLoop();

// �񎟌��z��N���X
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

// ��ԃN���X
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
		// ����`��
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
	// ���s�������������X�e�[�W�̕��A������ݒ�
	setSize(stageData, size);
	// setSize()�ŋ��߂����A���������ɔz����m��
	mObjects.setSize(mWidth, mHeight);
	mGoalFlags.setSize(mWidth, mHeight);
	// �T���v���ł͏������̏����������Ă��邪�A�����ď����Ȃ��B����
	// �G���[�N�������珑���Ă݂�
	for (int y = 0; y < mHeight; ++y) {
		for (int x = 0; x < mWidth; ++x) {
			mObjects(x, y) = OBJ_WALL;
			mGoalFlags(x, y) = false;
		}
	}

	// Array2D <Object>& o = mObjects; ���ꂢ��Ȃ�����
	Object t;
	bool goalFlag = false;
	int x = 0;
	int y = 0;


	for (int i = 0; i < size; i++) {
		switch (stageData[i])
		{
		case '#': t = OBJ_WALL; break;
		case '.': t = OBJ_SPACE; break; goalFlag = true; // SPACE�ŗǂ��́H��
		case 'p': t = OBJ_MAN;  break;
		case 'o': t = OBJ_BLOCK; break;
		case ' ': t = OBJ_SPACE; break;
		case '\n': y++; x = 0; t = OBJ_UNKNOWN; break;
		default: t = OBJ_UNKNOWN; break;
		}
		if (t != OBJ_UNKNOWN) {
			mObjects(x, y) = t;
			mGoalFlags(x, y) = goalFlag;
			// �l������x�ɕ��͍L���邩��Ax���C���N�������g
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
			// ���A�����̍ő�l���X�V
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
			case OBJ_WALL: color = 0xffffff; break; // �ǂ͔�
			case OBJ_SPACE: 
				if (g == true) {
					color = 0x00ffff; // �S�[���͗�
					break;
				}
				else {
					color = 0x000000; // �X�y�[�X�͍�
					break;
				}
			case OBJ_MAN: color = 0xff0000; break; // �v���C���[�͂͐�
			case OBJ_BLOCK: color = 0x0000ff; break; // �u���b�N�͐�
				// case OBJ_UNKNOWN: y++; x = 0; break; ����͂��͂⑶�݂��Ȃ�
			}
				drawCell(x, y, color);
		}
	}
}

void State::update(char input) {
	// �������߂�
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
