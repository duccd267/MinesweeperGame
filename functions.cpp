#include "main.h"
#include "functions.h"
#include "Console.h"
#include<iostream>
#include<vector>
#include<algorithm>
#include<fstream>
#include<sstream>
#include<cstring>

using namespace std;

// Khai báo 1 bảng (Table) và 1 con trỏ cấp 2 (Box) để lưu n^2 ô
TableStruct Table;
BoxStruct** Box;

// Vị trí con trỏ hiện tại
COORD CCurLocation;

// cờ sử dụng phím
bool BUseKeyboard = false;

// cờ sử dụng chuột
bool BUseMouse = false;

// biến global để lưu tọa độ cho lúc chuyển từ chuột sang phím
short x, y;

// Tạo độ x, y vẽ bảng
short SXCoord;
short SYCoord;

// Cập nhật trạng thái chơi game
bool BPlayGameStatus = false;

// Thời gian vào game  == điểm
int ITime = 0;

// Hàm tạo bảng (tạo 1 mảng 2 chiều)
void matrixCreate()
{
	Box = new BoxStruct * [Table.SRow];
	for (int i = 0; i < Table.SRow; ++i)
	{
		Box[i] = new BoxStruct[Table.SCol];
	}
}

// Hàm xóa bảng (xóa mảng 2 chiều theo thứ tự ngược lại so với tạo bảng)
void matrixDelete()
{
	for (int i = 0; i < Table.SRow; ++i)
	{
		delete[] Box[i];
	}
	delete[] Box;
}

// Hàm chuẩn hóa tạo độ x, y vẽ bảng
// Tính khoảng cách SXCoord, SYCoord của bảng chính là khoảng cách mà tọa độ gốc cần cộng thêm đẻ căn bảng vào giữa
// Tạo độ để vẽ các ô thực tế là giá trị của 2 hàm xCoord và yCoord
void tableCoordSave()
{
	SXCoord = ((ConsoleWidth / 2) - Table.SRow);
	SYCoord = (((ConsoleHeight - 6) - Table.SCol) / 2) + 7;
}

// Hàm khởi tạo
// Khởi tạo các giá trị cần thiết cho từng level được chọn
//    như là số dòng, số cột, số bom
void init(short SRow, short SCol, short SMineCount)
{
	Table.SRow = SRow;
	Table.SCol = SCol;
	Table.SMineCount = SMineCount;
	Table.SOpenOCount = 0;
	Table.SFlagCount = 0;
	if (SRow == 9) Table.SSuggest = 2;
	if (SRow == 16) Table.SSuggest = 5;
	if (SRow == 24) Table.SSuggest = 9;


	matrixCreate();
	randomMineCreate();
	tableCoordSave();
	CCurLocation = { 0, 0 };
	BPlayGameStatus = true;
	ITime = GetTickCount64();

	drawTable();
	drawPlayGameStatus(1, 0, 0);
}

short xCoord(short SX) // Toa do x ve bang.
{
	return ((SX * 2) + SXCoord);
}

short yCoord(short SY) // Toa do y ve bang.
{
	return (SY + SYCoord);
}

// Hàm sinh bom ngẫu nhiên
// Số bom cần sinh ra = số bom khi chọn level = SMineCount
// Dùng vòng lặp while, mỗi lần lặp sinh được thêm 1 bom và số bom cần sinh giảm đi 1
void randomMineCreate()
{
	short SSoBom = Table.SMineCount;
	short SI, SJ; // SI vi tri dong, SJ vi tri cot ta se random.
	srand(time(NULL)); // Reset time.
	while (SSoBom)
	{
		SI = rand() % Table.SRow;
		SJ = rand() % Table.SCol;
		if (Box[SI][SJ].BLandMine)
			continue;

		Box[SI][SJ].BLandMine = true;
		--SSoBom; // Cap nhat lai so luong bom.
	}
}

// Hàm cắm cờ
// Truyền vào tạo độ của ô cần cắm (SX, SY)
// Nếu chưa cắm thì cắm, nếu đã cắm thì sẽ bỏ cắm cờ, đồng thời cập nhật lại số lượng cờ
void clickRight(short SX, short SY) // Cam co. phím x
{

	if (SX >= 0 && SX < Table.SCol && SY >= 0 && SY < Table.SRow) {
		if (!Box[SX][SY].BOpened && BPlayGameStatus)
		{
			if (Box[SX][SY].BFlag)
			{
				Box[SX][SY].BFlag = false;
				Table.SFlagCount--;
			}
			else
			{
				Box[SX][SY].BFlag = true;;
				Table.SFlagCount++;
			}

			AUDIO(IDR_WAVE2);
		}
		drawTable();

		deleteRow(4, 1);
		drawPlayGameStatus(1, 0, 0);  // 1 là dang choi
	}
}

// Hàm gợi ý
// Mỗi mức có số lượng lần gợi ý khác nhau
// Ấn S để đượcc gọi ý, sau mỗi lần bấm, số gợi ý sẽ giảm đi 1
void suggest()
{
	if (Table.SSuggest > 0)
	{
		short i = 0;
		short j = 0;
		short size = Table.SCol;
		bool BBreak = false;
		for (i = 0; i < size; ++i) 
		{
			for (j = 0; j < size; ++j)
			{
				if (Box[i][j].BLandMine && !Box[i][j].BFlag && !Box[i][j].BOpened && BPlayGameStatus)
				{
					Box[i][j].BFlag = true;
					Table.SFlagCount++;
					drawTable();
					deleteRow(4, 1);
					drawPlayGameStatus(1, 0, 0);
					BBreak = true;
					//clickRight(i, j);
					break;
				}
			}
			if (BBreak) break;

		}
	}
	Table.SSuggest--;
}


// Hàm đến số bom lân cận của 1 ô
// Truyền vào tọa độ của ô đó (SX, SY)
// Duyệt qua các ô lân cận của ô đó để đếm, cuối cùng trả về giá trị của biến đếm SDem
short neighborMineCount(short SX, short SY)
{
	short SDem = 0;
	for (int i = SX - 1; i <= SX + 1; ++i)
	{
		for (int j = SY - 1; j <= SY + 1; ++j)
		{
			// Xet nhung vi tri khong hop => tiep tuc lap.
			if (i < 0 || i >= Table.SRow || j < 0 || j >= Table.SCol || (i == SX && j == SY))
			{
				continue;
			}

			// Xet xem o co bom hay khong. Co tang len dem 1.
			if (Box[i][j].BLandMine)
			{
				++SDem;
			}
		}
	}
	return SDem;
}

// Hàm mở 1 ô cụ thể
// Truyền vào tọa độ của ô cần mở (SX, SY)
// Nếu ô đó có bom thì mở ra lose() thua
// Nếu ô đó com bom lân cận thì mở ô và hiện số bom lân cận của ô đó
// Nếu ô đó không có bom lân cận thì gọi lại hàm mở ô đối với các ô xung quanh (đệ quy),
//		sử dụng biến BVisited (đã duyệt qua) để tránh duyệt lại những ô đã duyệt
void boxOpen(short SX, short SY)//them co da xet --> tranh de quy qua nhieu, quy uoc dat ten bien
{
	
	if (!Box[SX][SY].BOpened && !Box[SX][SY].BFlag)
	{
		Box[SX][SY].BOpened = true;
		Box[SX][SY].BVisited = true;
		if (Box[SX][SY].BLandMine) // O co boom.
		{
			lose(); // => thua.
		}
		else
		{
			Table.SOpenOCount++;
			short SSoBomLanCan = neighborMineCount(SX, SY);
			if (SSoBomLanCan) // Co bom lan can.
			{
				Box[SX][SY].SNeighborMine = SSoBomLanCan;
			}
			else // O rong.
			{
				// Duyet cac o lan can va goi de quy.
				for (int i = SX - 1; i <= SX + 1; ++i)
				{
					for (int j = SY - 1; j <= SY + 1; ++j)
					{
						// Xet nhung vi tri khong hop le => tiep tuc lap.
						if (i < 0 || i >= Table.SRow || j < 0 || j >= Table.SCol || Box[i][j].BVisited)
						{
							continue;
						}

						// Goi de quy.
						else {
							 boxOpen(i, j);
						}
					}
				}
			}
		}
	}

}

// Hàm kiểm tra xem đã win chưa
// bằng cách đếm số ô đã mở + số bom = n^2 = số dòng * số cột hay không
// Đã win trả về true, chưa win trả về false
bool flagStatistic()
{
	return ((Table.SOpenOCount + Table.SMineCount) == (Table.SRow * Table.SCol));
}

// Hàm ở ô
// Nếu ô đó chưa mở, chưa cắm cờ, game vẫn đang chơi thì gọi đến hàm mở ô boxOpen()
// Sau khi mở ô kiểm tra tiếp nếu thắng thì mở ra win(), nếu chưa thắng thì phát âm thanh cho hành động mở ô vừa rồi
void clickLeft(short SX, short SY)
{
	if (SX >= 0 && SX < Table.SCol && SY >= 0 && SY < Table.SRow) {
		if (!Box[SX][SY].BOpened && !Box[SX][SY].BFlag && BPlayGameStatus)
		{
			boxOpen(SX, SY);

			if (BPlayGameStatus)
			{
				drawTable();
				if (flagStatistic())
				{
					win();
				}
				else
				{
					AUDIO(IDR_WAVE1);
				}
			}
		}
	}
}

// Hàm xử lý trạng thái thắng
// Cập nhật trạng thái game, xóa con trỏ cấp phát
// Cuối cùng là phát ra 1 âm thanh win
void win()
{
	BPlayGameStatus = false;
	matrixDelete(); // Giai phong con tro.
	SPages = 5;
	deleteRow(4, 1);
	drawPlayGameStatus(2, 2, 0); // Cap nhat lai trang thai la thang.

	AUDIO(IDR_WAVE4);
}

// Hàm xử lý trạng thái thua
// Hiện các quả bom còn lại và kiểm tra cắm cờ dúng hay không
// Cập nhật trạng thái game, xóa con trỏ cấp phát
// Cuối cùng là phát ra 1 am thanh lose
void lose()
{
	// Hien bom an va kiem tra cam co dung hay sai.
	for (int i = 0; i < Table.SRow; ++i)
	{
		for (int j = 0; j < Table.SCol; ++j)
		{
			if (Box[i][j].BFlag) // Co cam co.
			{
				if (Box[i][j].BLandMine)
				{
					drawBox(j, i, 15); // Cam co dung(cam co bom).
				}
				else
				{
					drawBox(j, i, 14); // Cam co sai(cam co khong co bom).
				}
			}
			else // Khong co cam co.
			{
				if (Box[i][j].BLandMine) // Co bom -> Hien bom.
				{
					drawBox(j, i, 9); // Hien bom an.
				}
			}
		}
	}

	BPlayGameStatus = false;
	matrixDelete(); // Giai phong con tro.
	SPages = 4;
	deleteRow(4, 1);
	drawPlayGameStatus(3, 3, 0); // Cap nhat lai trang thai la thua.

	AUDIO(IDR_WAVE3);
}

// hàm xử lý chuột
// biến mouse là loại click chuột --> dùng switch để bắt sự kiện
// bắt sự kiện chuột được đưa vào --> bắt vị trí --> xử lý vị trí cho vào mảng 2 chiều để tương tác với từng Box
// case 0: 1 click chuột, nhấn chuột trái == mở ô, nhấn chuột phải = cắm cờ
void mouseProcessing(MOUSE_EVENT_RECORD mouse) { // xu ly chuot
#ifndef MOUSE_HWHEELED
#define MOUSE_HWHEELED 0x0008
#endif
	switch (mouse.dwEventFlags)
	{
	  case 0:
		if (mouse.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			BUseMouse = true;
			CCurLocation.Y = mouse.dwMousePosition.Y;
			CCurLocation.X = mouse.dwMousePosition.X;
			if (SPages == 3 && BPlayGameStatus)
			{
				x = (short)((CCurLocation.X - SXCoord) / 2);
				y = (short)((CCurLocation.Y - SYCoord));
				CCurLocation.X = x;
				CCurLocation.Y = y;
				clickLeft(y, x);
			}
		}
		else if (mouse.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
		{
			BUseMouse = true;
			CCurLocation.Y = mouse.dwMousePosition.Y;
			CCurLocation.X = mouse.dwMousePosition.X;
			if (SPages == 3 && BPlayGameStatus)
			{
				x = (short)((CCurLocation.X - SXCoord) / 2);
				y = (short)((CCurLocation.Y - SYCoord));
				CCurLocation.X = x;
				CCurLocation.Y = y;
				clickRight(y, x);
			}
		}

		break;

	}
}


// Hàm xử lý phím
// Truyền vào 1 biến cấu trúc KEY_EVENT_RECORD có kiểu dữ liệu thành phần là bool bKeyDown kiểm tra có nhấn phím hay không
// Dùng switch...case với điều kiện là biến wVirtualKeyCode là tên của 1 phím, mỗi trường hợp dùng để mô tả chức năng của từng phím sử dụng trong game
void keyboardProcessing(KEY_EVENT_RECORD key)
{
	if (key.bKeyDown) // Co nhan phim.
	{
		switch (key.wVirtualKeyCode)
		{
		case VK_UP: // Mui ten len.
			switch (SPages)
			{
			case 1: // Menu chinh.
				if (SSelectLocation == 1 || SSelectLocation == 0)
				{
					SSelectLocation = STotalCatalog;
				}
				else
				{
					SSelectLocation -= 1;
				}

				drawMainMenu(SSelectLocation);
				break;
			case 2: // Menu chon cap do.
				if (STotalCatalog == 4)
				{
					if (SSelectLocation == 0)
					{
						SSelectLocation = STotalCatalog - 1;
					}
					else
					{
						SSelectLocation -= 1;
					}

					drawLevelMenu(SSelectLocation);
				}
				break;
			case 3: // Trang choi game.
				if (BPlayGameStatus)
				{
					BUseKeyboard = true;
					CCurLocation.Y = ((CCurLocation.Y == 0) ? Table.SRow - 1 : CCurLocation.Y - 1);
					drawTable();
				}
				break;
			case 4: // Trang thua.
				drawPlayGameStatus(3, 3, (SSelectLocation == 0) ? 1 : 0);
				break;
			case 5: // Trang thang.
				drawPlayGameStatus(2, 2, (SSelectLocation == 0) ? 1 : 0);
				break;
			case 6: // Trang luu lai.
				drawPlayGameStatus(1, 1, (SSelectLocation == 0) ? 1 : 0);
				break;
			}
			break;
		case VK_DOWN: // Mui ten xuong.
			switch (SPages)
			{
			case 1: // Menu chinh.
				if (SSelectLocation == STotalCatalog)
				{
					(STotalCatalog == 4) ? SSelectLocation = 1 : SSelectLocation = 0;
				}
				else
				{
					SSelectLocation += 1;
				}
				drawMainMenu(SSelectLocation);
				break;
			case 2: // Menu chon cap do.
				if (STotalCatalog == 4)
				{
					if (SSelectLocation == STotalCatalog - 1)
					{
						SSelectLocation = 0;
					}
					else
					{
						SSelectLocation += 1;
					}

					drawLevelMenu(SSelectLocation);
				}
				break;
			case 3: // Trang choi game.
				if (BPlayGameStatus)
				{
					BUseKeyboard = true;
					CCurLocation.Y = ((CCurLocation.Y == Table.SRow - 1) ? 0 : CCurLocation.Y + 1);
					drawTable();
				}
				break;
			case 4: // Trang thua.
				drawPlayGameStatus(3, 3, (SSelectLocation == 0) ? 1 : 0);
				break;
			case 5: // Trang thang.
				drawPlayGameStatus(2, 2, (SSelectLocation == 0) ? 1 : 0);
				break;
			case 6: // Trang luu lai
				drawPlayGameStatus(1, 1, (SSelectLocation == 1) ? 0 : 1);
				break;
			}
			break;
		case VK_LEFT: // Mui ten trai.
			if (BPlayGameStatus)
			{
				BUseKeyboard = true;
				CCurLocation.X = ((CCurLocation.X == 0) ? Table.SCol - 1 : CCurLocation.X - 1);
				drawTable();
			}
			break;
		case VK_RIGHT: // Mui ten phai.
			if (BPlayGameStatus)
			{
				BUseKeyboard = true;
				CCurLocation.X = ((CCurLocation.X == Table.SCol - 1) ? 0 : CCurLocation.X + 1);
				drawTable();
			}
			break;
		case VK_RETURN: // Phim Enter.
			switch (SPages)
			{
			case 1: // Menu chinh.
				if (SSelectLocation == 0)
				{

				}
				else if (SSelectLocation == 1)
				{
					SPages = 2;
					deleteRow(4, 5);
					drawLevelMenu(0);
				}
				else if (SSelectLocation == 2) // Trang bang diem.
				{
					SPages = 7;
					deleteRow(4, 10);
					drawHighScore(0);
				}
				else if (SSelectLocation == 3) // Trang thong tin.
				{
					SPages = 8;
					deleteRow(4, 10);
					information(0);
				}
				else
				{
					exit(0);
				}
				break;
			case 2: // Menu chon cap do.
				if (SSelectLocation == 0) // Muc de 9 * 9 va 10 bom.
				{
					SPages = 3; // Cap nhat lai la dang choi game.
					deleteRow(4, 10);
					init(9, 9, 10);
				}
				else if (SSelectLocation == 1) // Muc trung binh 16 * 16 va 40 bom.
				{
					SPages = 3; // Cap nhat lai la dang choi game.
					deleteRow(4, 7);
					init(16, 16, 40);
				}
				else if (SSelectLocation == 2) // Muc kho 24 * 24 va 99 bom.
				{
					SPages = 3; // Cap nhat lai la dang choi game.
					deleteRow(4, 10);
					init(24, 24, 99);
				}
				else
				{
					SPages = 1; // Cap nhat lai la menu chinh.
					deleteRow(4, 10);
					drawMainMenu(0);
				}
				break;
			case 4: // Trang thua.
				if (SSelectLocation)
				{
					Table.STime = 0;
					SPages = 1; // Tro ve menu chinh.
					deleteRow(3, ConsoleHeight - 3);
					drawMainMenu(0);
				}
				else
				{
					Table.STime = 0;
					SPages = 3; // Trang choi game.
					deleteRow(3, ConsoleHeight - 3);
					init(Table.SRow, Table.SCol, Table.SMineCount);
				}
				break;
			case 5: // Trang thang.

				if (SSelectLocation)
				{
					Table.STime = 0;
					SPages = 1; // Tro ve menu chinh.
					deleteRow(3, ConsoleHeight - 3);
					drawMainMenu(1);
				}
				else
				{
					saveScore();
					Table.STime = 0;
					SPages = 1; // Tro ve menu chinh.
					deleteRow(3, ConsoleHeight - 3);
					drawMainMenu(1);
				}
				break;

			case 6: // Trang luu lai.
				if (SSelectLocation)
				{
					SPages = 1; // Tro ve menu chinh.
					deleteRow(3, ConsoleHeight - 3);
					drawMainMenu(1);
				}
				else
				{
					// Luu game -> Xu file.
				}
				break;
			case 7:
				if (SSelectLocation == 0)
				{
					SPages = 1;
					deleteRow(4, ConsoleHeight - 3);
					drawMainMenu(1);
					break;
				}
				break;
			case 8:
				if (SSelectLocation == 0)
				{
					SPages = 1;
					deleteRow(4, ConsoleHeight - 3);
					drawMainMenu(1);
					break;
				}
				break;
			}
			break;

		case VK_ESCAPE: // Phim ESC(thoat).
			switch (SPages)
			{
			case 1: // Menu chinh.
				exit(0);
				break;
			case 2: // Menu chon cap do.
				SPages = 1; // Cap nhat lai thanh trang menu chinh.
				deleteRow(4, 10);
				drawMainMenu(0);
				break;
			case 3: // Dang choi game.
				if (BPlayGameStatus)
				{
					BPlayGameStatus = false;
					SPages = 1;
					deleteRow(3, ConsoleHeight - 3);
					//drawPlayGameStatus(1, 1, 0);
					Table.STime = 0;
					drawMainMenu(1);
				}
				break;
			case 4: // Trang thua.
			case 5: // Trang thang.
				SPages = 2;
				deleteRow(3, ConsoleHeight - 3);
				drawLevelMenu(0);
				break;
			case 6: // Trang luu lai.
				if (!BPlayGameStatus)
				{
					BPlayGameStatus = false;
					SPages = 5;

					deleteRow(3, 2);
					drawPlayGameStatus(1, 0, 0);
				}
				break;
			case 7:
				SPages = 1; // Cap nhat lai thanh trang menu chinh.
				deleteRow(4, ConsoleHeight - 3);
				drawMainMenu(1);
				break;
			case 8:
				SPages = 1; // Cap nhat lai thanh trang menu chinh.
				deleteRow(4, ConsoleHeight - 3);
				drawMainMenu(1);
				break;
			} break;

		case ClickLeft: // Phim Z - Mo O.
			if (SPages == 3 && BPlayGameStatus)
				clickLeft(CCurLocation.Y, CCurLocation.X);
			break;
		case ClickRight: // Phim X - Cam Co.
			if (SPages == 3 && BPlayGameStatus)
				clickRight(CCurLocation.Y, CCurLocation.X);
			break;

		case Suggest: // phim S - goi y
			if (SPages == 3 && BPlayGameStatus)
				suggest();
			break;
		}
	}
}

// Hàm xử lý sự kiện đầu vào
// Đặt tất cả các sự kiện đọc được vào con trỏ IREventBuffer
// Duyệt qua tất cả các sự kiện (nếu có) bằng vòng lặp for và thực hiện từng sự kiện
void eventProcessing()
{
	while (1)
	{
		DWORD DWNumberOfEvents = 0; // Luu lai su kien hien tai.
		DWORD DWNumberOfEventsRead = 0; // Luu lai so luong su kien da duoc loc.
		DWORD fdwMode;

		HANDLE HConsoleInput = GetStdHandle(STD_INPUT_HANDLE); // Thiet bi dau vao.
		fdwMode = ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
		SetConsoleMode(HConsoleInput, fdwMode);
		GetNumberOfConsoleInputEvents(HConsoleInput, &DWNumberOfEvents); // Dat su kien dau vao cua giao dien cho bien DWNumberOfEvents.

		if (DWNumberOfEvents)
		{
			INPUT_RECORD* IREventBuffer = new INPUT_RECORD[DWNumberOfEvents]; // Con tro EventBuffer.
			ReadConsoleInput(HConsoleInput, IREventBuffer, DWNumberOfEvents, &DWNumberOfEventsRead); // Dat cac su kien duoc luu tru vao con EventBuffer.

			// Chay vong lap de doc su kien.
			for (DWORD i = 0; i < DWNumberOfEvents; ++i)
			{
				if (IREventBuffer[i].EventType == KEY_EVENT) // Neu la su kien phim.
				{
					keyboardProcessing(IREventBuffer[i].Event.KeyEvent);
				}
				else if (IREventBuffer[i].EventType == MOUSE_EVENT) // Su kien chuot.
				{
					mouseProcessing(IREventBuffer[i].Event.MouseEvent);
				}
			}
		}

		if (BPlayGameStatus && ((Table.SOpenOCount > 0) || (Table.SFlagCount > 0)))
		{
			int ITemp = ITime + 1000;
			if (GetTickCount64() > ITemp)
			{
				ITime = GetTickCount64();
				Table.STime++;

				drawPlayGameStatus(1, 0, 0);
			}
		}
	}
}

// Hàm vẽ tiêu đề game, vẽ từng thành phàn của tiêu đề phía trên
// Vẽ bằng cách sử dụng print và hàm trong thư viện Console.h
// setBackgroundTextXY(), setColor
void drawGameTitle()
{
	short i;
	for (i = 0; i < ConsoleWidth; ++i)
	{
		printf("%c", 45);
	}
	LPSTR StrTitle = (char*)"Game Do Min - Nhom 1 CPP\n";
	setBackgroundColorTextXY((ConsoleWidth / 2) - (strlen(StrTitle) / 2), 1, 5, 0, StrTitle);
	setColor(7);
	for (i = 0; i < ConsoleWidth; ++i)
	{
		printf("%c", 45);
	}
}

// Hàm vẽ Ô
// Truyền vào tọa độ X, Y, và biến short quy định đồng thời màu nền và màu chữ (có 16 trường hợp của STypes từ 0 đến 15)
// Mỗi trường hợp đều dùng hàm setBackgroundColorTextXY (được định nghĩa trong thư viện Console.h) để vẽ Ô 
void drawBox(short SX, short SY, short STypes)
{
	switch (STypes)
	{
	case 0: // Rong mau xanh la.
		setBackgroundColorTextXY(xCoord(SX), yCoord(SY), 9, 15, (char*)"  ");
		break;
	case 1: // So 1 xanh duong. So 1 -> 8 la nen trang.
		setBackgroundColorTextXY(xCoord(SX), yCoord(SY), 9, 15, (char*)"1 ");
		break;
	case 2: // So 2 xanh la.
		setBackgroundColorTextXY(xCoord(SX), yCoord(SY), 2, 15, (char*)"2 ");
		break;
	case 3: // So 3 do.
		setBackgroundColorTextXY(xCoord(SX), yCoord(SY), 12, 15, (char*)"3 ");
		break;
	case 4: // So 4 xanh duong dam.
		setBackgroundColorTextXY(xCoord(SX), yCoord(SY), 1, 15, (char*)"4 ");
		break;
	case 5: // So 5 do dam.
		setBackgroundColorTextXY(xCoord(SX), yCoord(SY), 4, 15, (char*)"5 ");
		break;
	case 6: // So 6 CYAN dam.
		setBackgroundColorTextXY(xCoord(SX), yCoord(SY), 3, 15, (char*)"6 ");
		break;
	case 7: // So 7 den
		setBackgroundColorTextXY(xCoord(SX), yCoord(SY), 0, 15, (char*)"7 ");
		break;
	case 8: // So 8 hong.
		setBackgroundColorTextXY(xCoord(SX), yCoord(SY), 13, 15, (char*)"8 ");
		break;
	case 9: // Bom. Nen do, chu den.
		setBackgroundColorTextXY(xCoord(SX), yCoord(SY), 0, 12, (char*)"B ");
		break;
	case 10: // O chan.
		setBackgroundColorTextXY(xCoord(SX), yCoord(SY), 0, 8, (char*)"  ");
		break;
	case 11: // O le.
		setBackgroundColorTextXY(xCoord(SX), yCoord(SY), 0, 7, (char*)"  ");
		break;
	case 12: // Theo doi con tro.
		setBackgroundColorTextXY(xCoord(SX) + 1, yCoord(SY), 0, 13, (char*)" ");
		break;
	case 13: // Cam co. Nen vang nhat, chu do.
		setBackgroundColorTextXY(xCoord(SX), yCoord(SY), 12, 14, (char*)"P ");
		break;
	case 14: // Cam co khong co bom => cam co sai. Nen cam, chu trang.
		setBackgroundColorTextXY(xCoord(SX), yCoord(SY), 15, 6, (char*)"Px");
		break;
	case 15: // Cam co co bom => cam co dung. Nen vang nhat, chu do.
		setBackgroundColorTextXY(xCoord(SX), yCoord(SY), 12, 14, (char*)"B ");
		break;
	}
}

// Hàm vẽ bảng
// Dùng 2 vòng lặp lồng nhau để vẽ từng ô của ma trận 
// Duyệt đến ô nào iểm tra các trạng thái ô (đã mở, cảm cờ, ..) để vẽ cho phù hợp
// và cuối cùng là vẽ con trỏ để theo dõi con trỏ
void drawTable()
{
	for (int i = 0; i < Table.SRow; ++i)
	{
		for (int j = 0; j < Table.SCol; ++j)
		{
			if (Box[i][j].BFlag)
				drawBox(j, i, 13);
			else if (Box[i][j].SNeighborMine)
				drawBox(j, i, Box[i][j].SNeighborMine);
			else if (Box[i][j].BOpened) // O rong.
				drawBox(j, i, 0);
			else if ((i + j) % 2) // O le.
				drawBox(j, i, 11);
			else // O chan.
				drawBox(j, i, 10);

			if (BUseKeyboard)
				drawBox(CCurLocation.X, CCurLocation.Y, 12);
		}
	}
}

// Hàm vẽ trạng thái chơi game, ngay phía dưới tiêu đề game
// Truyền vào 3 biến SStatus (trạng thái: đang chơi, thắng, thua), 
//  SOptions (lựa chọn khi thắng hoặc thua), SIndex (vị trí đang được chọn trên menu)
void drawPlayGameStatus(short SStatus, short SOptions, short SIndex)
{
	SSelectLocation = SIndex;
	STotalCatalog = 2;
	if (Table.SSuggest < 1) Table.SSuggest = 0;

	setBackgroundColorTextXY(1, 3, 15, 0, (char*)"Ban Do: %d * %d", Table.SRow, Table.SCol);
	setBackgroundColorTextXY(1, 4, 15, 0, (char*)"So Bom: %d", Table.SMineCount - Table.SFlagCount);
	setBackgroundColorTextXY(ConsoleWidth - 15, 3, 15, 0, (char*)"Thoi Gian: %d", Table.STime);
	setBackgroundColorTextXY(35, 3, 15, 0, (char*)"Goi Y: %d", Table.SSuggest);

	// Ve menu thang thua.
	LPSTR StrTextOptionMenu;
	if (SOptions == 1)
	{
		StrTextOptionMenu = (char*)"  Luu Lai  ";
		setBackgroundColorTextXY((ConsoleWidth / 2) - (strlen(StrTextOptionMenu) / 2), 3, 15, ((SIndex == 0) ? 2 : 0), StrTextOptionMenu);
	}
	if (SOptions == 2)
	{
		StrTextOptionMenu = (char*)"  Luu Diem  ";
		setBackgroundColorTextXY((ConsoleWidth / 2) - (strlen(StrTextOptionMenu) / 2), 3, 15, ((SIndex == 0) ? 2 : 0), StrTextOptionMenu);
	}
	if (SOptions == 3)
	{
		StrTextOptionMenu = (char*)"  Choi Lai  ";
		setBackgroundColorTextXY((ConsoleWidth / 2) - (strlen(StrTextOptionMenu) / 2) + 1, 3, 15, ((SIndex == 0) ? 2 : 0), StrTextOptionMenu);
	}

	if (SOptions >= 1)
	{
		StrTextOptionMenu = (char*)"  Thoat  ";
		setBackgroundColorTextXY((ConsoleWidth / 2) - (strlen(StrTextOptionMenu) / 2), 4, 15, ((SIndex == 1) ? 2 : 0), StrTextOptionMenu);
	}

	// Ve text trang thai.
	if (SStatus == 1) // 1 Dang choi game.
	{
		setBackgroundColorTextXY(ConsoleWidth - 22, 4, 15, 0, (char*)"Trang Thai: %s", "Dang Choi");
	}
	if (SStatus == 2) // 2 win.
	{
		setBackgroundColorTextXY(ConsoleWidth - 22, 4, 14, 0, (char*)"Trang Thai: %s", "Thang");
	}
	if (SStatus == 3) // 3 Thua
	{
		setBackgroundColorTextXY(ConsoleWidth - 22, 4, 12, 0, (char*)"Trang Thai: %s", "Thua");
	}
	std::cout << "\n";
	setColor(7);
	short i;
	for (i = 0; i < ConsoleWidth; ++i)
	{
		printf("%c", 45);
	}
}

// Hàm vẽ menu game chính
// Khai báo 2 biến chính, thứ nhất là SIndex chỉ vị trí đang chọn trong menu
//		thứ 2 là biến STotalCatalog lưu tổng số mục trong menu
// Dùng hàm trong Console.h để vẽ ra các dòng của menu
// Có 4 mục ứng với SIndex từ 1 đến 4
void drawMainMenu(short SIndex)
{
	// Cap nhat lai vi tri dang chon va tong muc cua menu.
	SSelectLocation = SIndex;
	STotalCatalog = 4;

	// Ve menu.
	LPSTR StrTextMainMenu = (char*)"  GAME MOI  ";
	setBackgroundColorTextXY((ConsoleWidth / 2) - (strlen(StrTextMainMenu) / 2), 7, 15, ((SIndex == 1) ? 2 : 0), StrTextMainMenu);

	StrTextMainMenu = (char*)"  BANG DIEM  ";
	setBackgroundColorTextXY((ConsoleWidth / 2) - (strlen(StrTextMainMenu) / 2), 8, 15, ((SIndex == 2) ? 2 : 0), StrTextMainMenu);

	StrTextMainMenu = (char*)"  THONG TIN  ";
	setBackgroundColorTextXY((ConsoleWidth / 2) - (strlen(StrTextMainMenu) / 2), 9, 15, ((SIndex == 3) ? 2 : 0), StrTextMainMenu);

	StrTextMainMenu = (char*)"  THOAT  ";
	setBackgroundColorTextXY((ConsoleWidth / 2) - (strlen(StrTextMainMenu) / 2), 10, 15, ((SIndex == 4) ? 2 : 0), StrTextMainMenu);
}

// Hàm vẽ menu cấp độ
// Tương tự như vẽ menu chính
void drawLevelMenu(short SIndex)
{
	// Cap nhat lai vi tri dang chon va tong muc cua menu.
	SSelectLocation = SIndex;
	STotalCatalog = 4;

	LPSTR StrTextLevelMenu = (char*)"CHON CAP DO";
	setBackgroundColorTextXY((ConsoleWidth / 2) - (strlen(StrTextLevelMenu) / 2), 4, 1, 0, StrTextLevelMenu);

	// Ve menu.
	StrTextLevelMenu = (char*)"  DE (9 * 9 VA 10 BOM)  ";
	setBackgroundColorTextXY((ConsoleWidth / 2) - (strlen(StrTextLevelMenu) / 2), 7, 15, ((SIndex == 0) ? 2 : 0), StrTextLevelMenu);

	StrTextLevelMenu = (char*)" TRUNG BINH (16 * 16 VA 40 BOM)  ";
	setBackgroundColorTextXY((ConsoleWidth / 2) - (strlen(StrTextLevelMenu) / 2), 8, 15, ((SIndex == 1) ? 2 : 0), StrTextLevelMenu);

	StrTextLevelMenu = (char*)" KHO (24 * 24 VA 99 BOM)  ";
	setBackgroundColorTextXY((ConsoleWidth / 2) - (strlen(StrTextLevelMenu) / 2), 9, 15, ((SIndex == 2) ? 2 : 0), StrTextLevelMenu);

	StrTextLevelMenu = (char*)"  QUAY LAI  ";
	setBackgroundColorTextXY((ConsoleWidth / 2) - (strlen(StrTextLevelMenu) / 2), 10, 15, ((SIndex == 3) ? 2 : 0), StrTextLevelMenu);
}


// Hàm lưu và hiển thị điểm cao
// Mỗi mức lưu vào 1 file, nếu thời gian thắng của mỗi mức nằm trong 
//  top 6 thì được hiển thị ở bảng điểm cao
void drawHighScore(short SIndex)
{
	vector<string> lines9;
	vector<string> lines16;
	vector<string> lines24;
	string line9x9, line16x16, line24x24;

	short rank = 0;

	ifstream ifile_9;
	ifile_9.open("file9x9.txt");
	while (getline(ifile_9, line9x9)) 
	{
		lines9.push_back(line9x9);
	}
	ifile_9.close();

	ifstream ifile_16;
	ifile_16.open("file16x16.txt");
	while(getline(ifile_16, line16x16))
	{
		lines16.push_back(line16x16);
	}
	ifile_16.close();

	ifstream ifile_24;
	ifile_24.open("file24x24.txt");
	while (getline(ifile_24, line24x24))
	{
		lines24.push_back(line24x24);
	}
	ifile_24.close();


	cout << "\n\n                               HIGH SCORE";
	cout << "\n\n\n\n   EASY (time in seconds):  ";
	for (const auto& i : lines9)
	{
		cout << (rank+1) <<"-"<<i << "s" << "  ";
		rank++;
	}
	rank = 0;
	cout << "\n\n   MEDIUM (time in seconds): ";
	for (const auto& i : lines16)
	{
		cout << (rank + 1) << "-" << i << "s" << "  ";
		rank++;
	}
	rank = 0;
	cout << "\n\n   HARD (time in seconds):  ";
	for (const auto& i : lines24)
	{
		cout << (rank + 1) << "-" << i << "s" << "  ";
		rank++;
	}
	cout << "\n\n\n";

	SSelectLocation = SIndex;
	STotalCatalog = 1;
	LPSTR StrTextHighScore = (char*)"  BACK  ";
	setBackgroundColorTextXY((ConsoleWidth / 2) - (strlen(StrTextHighScore) / 2), 17, 15, ((SIndex == 0) ? 2 : 0), StrTextHighScore);
}

// hàm lưu điểm : lưu lại thời gian khi đã thắng ván game
// mỗi mức chơi sẽ lưu vào 1 file khác nhau
// hàm sẽ mở file --> đọc dữ liệu trong file (6 điểm cao nhất)bằng vector
//  --> thêm thời gian mới hoàn thành vào cuối vector --> sắp xếp --> lưu lại vào file
void saveScore() {
	short time = Table.STime;
	short level = Table.SCol;
	short count = 6;
	vector<short> arr;
	ifstream read;
	if (level == 9)
	{
		read.open("file9x9.txt");
	}
	else if (level == 16)
	{
		read.open("file16x16.txt");
	}
	else
	{
		read.open("file24x24.txt");
	}
	
	if (read.is_open())
	{
		short i = -1;
		while (read >> i)
		{
			if (i == -1) break;
			arr.push_back(i);
		}
	}
	arr.push_back(time);
	sort(arr.begin(), arr.end());
	read.close();

	ofstream write;
	if (level == 9)
	{
		write.open("file9x9.txt", ios::trunc);
	}
	else if (level == 16)
	{
		write.open("file16x16.txt", ios::trunc);
	}
	else
	{
		write.open("file24x24.txt", ios::trunc);
	}


	if (write.is_open())
	{
		short size = arr.size();
		short j = 0;
		while (j < size && count > 0)
		{
			write << arr[j];
			write << "\n";
			++j;
			count--;
		}
	}
	
	write.close();
}


// đọc thông tin về game đã được lưu trong 1 file 
void information(short SIndex) 
{
	// Khai báo vector để lưu các dòng đọc được
	vector<string> lines;
	string line;

	//Mở file bằng ifstream
	ifstream input_file;
	input_file.open("HD.txt");
	//Kiểm tra file đã mở thành công chưa
	if (!input_file.is_open()) {
		cout << "Could not open the file "<< endl;
		//return 0;
	}

	//Đọc từng dòng trong
	while (getline(input_file, line)) {
		lines.push_back(line);//Lưu từng dòng như một phần tử vào vector lines.
	}

	//Đóng file
	input_file.close();

	//Xuất từng dòng từ lines và in ra màn hình
	for (const auto& i : lines)
		cout << i << endl;

	SSelectLocation = SIndex;
	STotalCatalog = 1;
	LPSTR StrTextHighScore = (char*)"  BACK  ";
	setBackgroundColorTextXY((ConsoleWidth / 2) - (strlen(StrTextHighScore) / 2), 25, 15, ((SIndex == 0) ? 2 : 0), StrTextHighScore);
	
}


