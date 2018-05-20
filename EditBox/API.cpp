#include "stdafx.h"
#include "Cursor.h"
#include <stack>


Cursor* pCursor = NULL;
wchar_t* pTChar = NULL;
wchar_t* pBuffer = NULL;
size_t MaxBufferSize = 0;
std::stack<Record*>* pRecord = NULL;
RVALUE __stdcall UserMessageProc(
	_In_ HTEXT hText,
	_In_opt_ int x, _In_opt_ int y,
	_In_ UINT message,
	_In_opt_ FPARAM fParam, _In_opt_ SPARAM sParam,
	_Out_opt_ LPKERNELINFO lpKernelInfo
)
{
	pCursor->SethText(hText);
	int Width_EN = Install::Width;			//�ַ����ȣ�Ӣ�ġ�������ŵ�λ���� ���֡���������ռ����Width��	
	int Width_ZH = 2 * Width_EN;			//����ģʽ�·��ſ���
	int Height = Install::Height;			//�ַ��߶�
	int LineNumber = y / Height + 1;
	//����������ı��·��հ�������кż���
	if (LineNumber > hText->Line_Number() && message != UM_TEXT)
	{
		LineNumber = hText->Line_Number();
		y = (LineNumber - 1) * Height;
	}
	//���y����
	y = y - y % Height;
	//����Ϣ�����Ĵ���
	switch (message)
	{
	case UM_UP:				//��
	{
		if (LineNumber > 1)
		{
			y -= Height;
			int LastLineWidth = hText->Line_Width(LineNumber - 1, Width_EN);
			x = min(LastLineWidth, x);
			if (!(pCursor->isLegalCursor(LineNumber - 1, x)))			//���λ�úϷ��Լ�� ��ֹ�������ƶ�ʹ����ں����м���ʾ
			{
				x -= Width_EN;
			}
		}
		pCursor->ResetChoose();
		/*���ù������λ�� ���к�*/
		lpKernelInfo->m_pCaretPixelPos = { x,y };
		lpKernelInfo->m_cCaretCoord = { (short)pCursor->Characters_before_Cursor(LineNumber,x),(short)LineNumber };
		break;
	}
	case UM_DOWN:			//��
	{
		if (LineNumber < hText->Line_Number())
		{
			y += Height;
			int NextLineWidth = hText->Line_Width(LineNumber + 1, Width_EN);
			x = min(NextLineWidth, x);
			if (!(pCursor->isLegalCursor(LineNumber + 1, x)))
			{
				x -= Width_EN;
			}
		}
		pCursor->ResetChoose();
		/*���ù������λ�� ���к�*/
		lpKernelInfo->m_pCaretPixelPos = { x,y };
		lpKernelInfo->m_cCaretCoord = { (short)pCursor->Characters_before_Cursor(LineNumber,x),(short)LineNumber };
		break;
	}
	case UM_LEFT:			//��
	{
		int opt = pCursor->CharactersProperty_before_Cursor(LineNumber, x);
		switch (opt)
		{
			case 1:x -= Width_ZH; break;
			case -1:x -= Width_EN; break;
			default:
			{
				if (LineNumber != 1)
				{
					x = hText->Line_Width(LineNumber - 1, Width_EN);
					y -= Height;
				}
				break;
			}
		}
		pCursor->ResetChoose();
		/*���ù������λ�� ���к�*/
		lpKernelInfo->m_pCaretPixelPos = { x,y };
		lpKernelInfo->m_cCaretCoord = { (short)pCursor->Characters_before_Cursor(LineNumber,x) ,(short)LineNumber };
		break;
	}
	case UM_RIGHT:			//��
	{
		int Size = pCursor->Characters_before_Cursor(LineNumber, x);
		if (Size == hText->Line_Size(LineNumber))
		{
			if (LineNumber != hText->Line_Number())		//��ͨ��β
			{
				y += Height;
				x = 0;
			}
		}
		else
		{
			if (pCursor->isEnAfter(LineNumber, x))
				x += Width_EN;
			else
				x += Width_ZH;
		}
		pCursor->ResetChoose();
		/*���ù������λ�� ���к�*/
		lpKernelInfo->m_pCaretPixelPos = { x,y };
		lpKernelInfo->m_cCaretCoord = { (short)pCursor->Characters_before_Cursor(LineNumber,x) ,(short)LineNumber };
		break;
	}
	case UM_END:			//��ǰ��βʱx y����
	{
		x = hText->Line_Width(LineNumber, Width_EN);
		pCursor->ResetChoose();
		/*���ù������λ�� ���к�*/
		lpKernelInfo->m_pCaretPixelPos = { x,y };
		lpKernelInfo->m_cCaretCoord = { (short)pCursor->Characters_before_Cursor(LineNumber,x),(short)LineNumber };
		break;
	}
	case UM_RETURN:			//�س�����
	{
		Position position;
		/*���ı��ڵĴ���*/
		position.LineNumber = LineNumber;
		position.Sequence = pCursor->Characters_before_Cursor(LineNumber, x);

		Record* rd = new Record(RD_RETURN);							//��¼������Ϣ
		rd->Set_Choose_Data(position, position);
		pRecord->push(rd);

		position = hText->EnterNewLine(position);
		int New_Width = hText->Max_Line_Width(Width_EN);			//�������ı�������


		pCursor->ResetChoose();
		/*���ù������λ�� ���к�*/
		lpKernelInfo->m_pCaretPixelPos = { 0,y + Height };
		lpKernelInfo->m_cCaretCoord = { (short)pCursor->Characters_before_Cursor(LineNumber,x),(short)LineNumber };
		/*���û�����Ϣ �ı���С*/
		lpKernelInfo->m_bLineBreak = TRUE;
		lpKernelInfo->m_pTextPixelSize = { hText->Max_Line_Width(Width_EN),hText->Line_Number()*Height };
		break;
	}
	case UM_DELETE:			//ɾ��
	{
		int Old_Lines = hText->Line_Number();
		Record* rd = new Record(RD_DELETE);
		//ѡ�ν�����������
		int end_x = LODWORD(fParam);
		int end_y = LODWORD(sParam);
		Position start_position;
		Position end_position;
		//ɾ��start������ַ���end���ǰ�������ַ�
		try
		{
			start_position = pCursor->CursorToPosition_After(x, y);		//���ɾ�������

		}
		catch (std::invalid_argument &e)
		{
			//���λ���ı�ĩβ
			delete rd;
			/*���û�����Ϣ �ı���С*/
			lpKernelInfo->m_bLineBreak = FALSE;
			lpKernelInfo->m_pTextPixelSize = { hText->Max_Line_Width(Width_EN),hText->Line_Number()*Height };
			break;
		}
		end_position = pCursor->CursorToPosition(end_x, end_y);
		//������ȱ�ʾ���ɾ��
		if (end_x == x && end_y == y)
		{
			try
			{
				end_position = pCursor->CursorToPosition_After(end_x, end_y);
				if (end_position.LineNumber != LineNumber)			//�л��е����  ��  ���λ����β
				{
					rd->ACT = RD_MERGE_LINE;
					rd->Save_Merge_Line_Data(hText, end_position.LineNumber);
					end_position.Sequence = 0;
				}
				else
				{
					rd->Save_Delete_Data(hText, end_position, end_position);
				}
				hText->BackSpace(end_position);
				pRecord->push(rd);
			}
			catch (std::invalid_argument& e)
			{
				//�յ��쳣 ˵����ǰ���λ���ı�ĩβ �����ɾ����������
				delete rd;
				pCursor->ResetChoose();
				/*���û�����Ϣ �ı���С*/
				lpKernelInfo->m_bLineBreak = FALSE;
				lpKernelInfo->m_pTextPixelSize = { hText->Max_Line_Width(Width_EN),hText->Line_Number()*Height };
				break;
			}
		}
		//λ����ͬ��ʾ�˸��
		else if (end_position <= start_position)
		{
			if (end_position.Sequence == 0)
			{
				rd->ACT = RD_MERGE_LINE;
				if (end_position.LineNumber > 1)			//����¼��Ч��ɾ��
				{
					rd->Save_Merge_Line_Data(hText, end_position.LineNumber);
					pRecord->push(rd);
				}
				else
					delete rd;
			}
			else
			{
				rd->Save_Delete_Data(hText, start_position, start_position);
				pRecord->push(rd);
			}
			hText->BackSpace(end_position);
		}
		else
		{
			rd->Save_Delete_Data(hText, start_position, end_position);
			pRecord->push(rd);
			hText->Delete(start_position, end_position);
		}

		pCursor->ResetChoose();
		int New_Lines = hText->Line_Number();
		/*���û�����Ϣ �ı���С*/
		lpKernelInfo->m_bLineBreak = (Old_Lines == New_Lines ? FALSE : TRUE);
		lpKernelInfo->m_pTextPixelSize = { hText->Max_Line_Width(Width_EN),hText->Line_Number()*Height };
		break;
	}
	case UM_CURSOR:			//��λ�Ϸ����
	{
		x = pCursor->CursorLocation(LineNumber, x);
		pCursor->ResetChoose();

		/*���ù������λ�� ���к�*/
		lpKernelInfo->m_pCaretPixelPos = { x,y };
		lpKernelInfo->m_cCaretCoord = { (short)pCursor->Characters_before_Cursor(LineNumber,x) ,(short)LineNumber };
		break;
	}
	case UM_HOME:		
	{
		x = 0;
		pCursor->ResetChoose();
		/*���ù������λ�� ���к�*/
		lpKernelInfo->m_pCaretPixelPos = { x,y };
		lpKernelInfo->m_cCaretCoord = { (short)pCursor->Characters_before_Cursor(LineNumber,x), (short)LineNumber };
		break;
	}
	case UM_NEW:
	{
		if (hText->isSaved())
		{
			hText->ClearAll();
			pCursor->ResetChoose();
			return UR_SAVED;
		}
		return  UR_NOTSAVED;
	}
	case UM_SAVE:
	{
		if (hText->File_Name().empty())
		{
			hText->Set_File_Name(Generate_Default_File_Name(CText::Path));
			std::wstring FileName = StringToWString(hText->File_Name());
			Alloc_Buffer(pBuffer, MaxBufferSize, FileName.size());
			WStringToWch(FileName, pBuffer);
			lpKernelInfo->m_lpchText = pBuffer;
			return UR_DEFAULTPATH;
		}
		else
		{
			hText->Set_File_Name(wchTostring((TCHAR*)sParam));
			hText->Save();
		}
		break;
	}
	case UM_OPEN:
	{
		std::string FileName = wchTostring((TCHAR*)sParam);
		try
		{
			hText->ReadText(FileName);
		}
		catch (Read_Text_Failed e)
		{
			return UR_ERROR;
		}
		pCursor->ResetChoose();
		/*���� �ı���С */
		lpKernelInfo->m_pTextPixelSize = { hText->Max_Line_Width(Width_EN),hText->Line_Number()*Height };
		break;
		
	}
	case UM_COPY:
	{
		if (!pCursor->isChoose())
			return UR_ERROR;
		Position copy_start = pCursor->start;
		Position copy_end = pCursor->end;
		std::wstring wstr_copy = hText->Copy(copy_start, copy_end);
		Alloc_Buffer(pBuffer, MaxBufferSize, wstr_copy.size());
		WStringToWch(wstr_copy, pBuffer);
		if (wstr_copy.size() < MaxBufferSize)
			pBuffer[wstr_copy.size()] = L'\0';
		lpKernelInfo->m_lpchText = pBuffer;
		lpKernelInfo->m_uiCount = wstr_copy.size();
		break;
	}
	case UM_CHAR:
	{
		Record* rd = new Record(RD_INSERT);
		std::wstring ws;
		TCHAR wch = (TCHAR)LODWORD(sParam);			//�������ַ�
		int Old_Lines = hText->Line_Number();
		//����ѡ����Ϣ �������Ϊ�滻����
		if (pCursor->isChoose())
		{
			ws.push_back(wch);
			rd->ACT = RD_REPLACE;
			rd->Save_Delete_Data(hText, pCursor->start, pCursor->end);	

			hText->Replace(pCursor->start, pCursor->end, ws);

			rd->Set_Choose_Data(pCursor->start, pCursor->start);
			x = (pCursor->PositionToCursor(pCursor->start)).x;
			pCursor->ResetChoose();
		}
		else
		{
			Position position = pCursor->CursorToPosition(x, y);
			if (wch == L'\t')							//����Tab
			{
				int PreWidth = hText->Line_Width(position.LineNumber, Width_EN, position.Sequence);
				int n = TAB_SIZE - (PreWidth / Width_EN) % TAB_SIZE;			//n��ʾ��Ҫ����ո������
				x += Width_EN * n;
				for (int i = 0; i < n; i++)
					ws.push_back(L' ');
				Position temp = { position.LineNumber,position.Sequence + 1 };
				position = hText->Insert(position, ws);
				rd->Set_Choose_Data(temp, position);
			}
			else
			{
				ws.push_back(wch);

				position = hText->Insert(position, ws);				//��Position�������һ���ַ�  position��¼�������ַ�λ��

				rd->Set_Choose_Data(position, position);
				if (WORD(wch >> 8) > 0)
					x += Width_ZH;
				else
					x += Width_EN;
			}
		}
		pRecord->push(rd);
		int New_Lines = hText->Line_Number();
		/*���ù������λ�� ���к�*/
		lpKernelInfo->m_pCaretPixelPos = { x,y };
		lpKernelInfo->m_cCaretCoord = { (short)pCursor->Characters_before_Cursor(LineNumber,x) , (short)LineNumber };
		/*���û�����Ϣ �ı���С*/
		lpKernelInfo->m_bLineBreak = (Old_Lines == New_Lines ? FALSE : TRUE);
		lpKernelInfo->m_pTextPixelSize = { hText->Max_Line_Width(Width_EN),hText->Line_Number()*Height };
		break;
	}
	case UM_TEXT:
	{
		lpKernelInfo->m_lpchText = pTChar;
		//���ı�����
		if (hText->Line_Number() < LineNumber)	
		{
			pTChar[0] = L'\0';
			lpKernelInfo->m_uiCount = 0;
			break;
		}
		pCursor->CursorLocation(LineNumber, x);

		int iCount = 0;					//��ʾ���ַ�����
		short int iStart = 0;			//�������ֵĿ�ʼ��
		short int iEnd = 0;				//�������ֽ�����

		int end_x = fParam;
		CLine* pLine = hText->GetLinePointer(LineNumber);
		Position position_start;		//�ַ���ʾ���
		Position position_end;			//�ַ���ʾ�յ�
		try
		{
			position_start = pCursor->CursorToPosition_After(x, y);		//��ù����ַ�
		}
		catch (std::invalid_argument& e)
		{
			//��ʼ���λ���ı�ĩβ
			pTChar[0] = L'\0';
			lpKernelInfo->m_uiCount = 0;
			break;
		}
		end_x = pCursor->CursorLocation(LineNumber, end_x);
		position_end = pCursor->CursorToPosition(end_x, y);
		std::wstring WStr = pLine->TransformToWString(position_start.Sequence, position_end.Sequence);
		WStringToWch(WStr, pTChar);
		iCount = WStr.size();
		if (iCount < TEXT_SIZE)
			pTChar[iCount] = L'\0';
		//���ø�������  ��0��ʼ
		if (pCursor->isChoose())
		{
			Set_Height_Light(position_start.LineNumber, position_start, position_end, iStart, iEnd);
			lpKernelInfo->m_pStartPixelPos = pCursor->PositionToCursor_Before(pCursor->start);
			lpKernelInfo->m_pEndPixelPos = pCursor->PositionToCursor(pCursor->end);
		}	

		lpKernelInfo->m_uiCount = iCount;
		lpKernelInfo->m_uiStart = iStart;
		lpKernelInfo->m_uiEnd = iEnd;
		lpKernelInfo->m_bInside = !pCursor->isLegalCursor(LineNumber, x);
		break;
	}
	case UM_ALL:
	{
		pCursor->Choose(hText->First_Position(), hText->End_Position());		//ȫѡ
		/*���ù������λ�� ���к�*/
		lpKernelInfo->m_pCaretPixelPos = { 0,0 };
		lpKernelInfo->m_cCaretCoord = { 0,1 };
		break;
	}
	case UM_CHOOSE:
	{
		Position start, end;
		int end_x = fParam;
		int end_y = sParam;
		end_y -= end_y % Height;
		end_x = pCursor->CursorLocation(end_y / Height + 1, end_x);
		x = pCursor->CursorLocation(LineNumber, x);
		POINT p = { x,y };
		if (y > end_y)
		{
			std::swap(x, end_x);
			std::swap(y, end_y);
		}
		else if (y == end_y && x > end_x)
			std::swap(x, end_x);
		
	
		end = pCursor->CursorToPosition(end_x, end_y);
		try
		{
			start = pCursor->CursorToPosition_After(x, y);
			pCursor->Choose(start, end);					//��¼ѡ����Ϣ ��start ��ǰ end �ں�
			if (start > end)
				throw std::invalid_argument("��Ч��ѡ��");
		}
		catch (std::invalid_argument& e)
		{
			pCursor->ResetChoose();
		}				

		/*���ù������λ�� ���к�*/
		lpKernelInfo->m_pCaretPixelPos = { x,y };
		lpKernelInfo->m_cCaretCoord = { (short)pCursor->Characters_before_Cursor(LineNumber,x),(short)LineNumber };
		break;
	}
	case UM_CANCEL:
	{

		if (pRecord->empty())			//��ǰû�д������Ĳ���
			return UR_NOTCANCEL;
		Record* p = pRecord->top();
		int Old_LineNumbers = hText->Line_Number();
		//�������һ�εĲ���
		p->ReDo(hText);
		pCursor->ResetChoose();			//���֮ǰ��ѡ����Ϣ
		switch (p->ACT)
		{
			case RD_MERGE_LINE:
			{
				lpKernelInfo->m_pCaretPixelPos = pCursor->PositionToCursor(p->start);
				lpKernelInfo->m_cCaretCoord = { (short)p->start.Sequence,(short)p->start.LineNumber };
				break;
			}
			case RD_INSERT :
			{
				lpKernelInfo->m_pCaretPixelPos = pCursor->PositionToCursor(p->start);
				lpKernelInfo->m_cCaretCoord = { (short)p->start.Sequence,(short)p->start.LineNumber };
				break;
			}
			case RD_RETURN :
			{
				lpKernelInfo->m_pCaretPixelPos = pCursor->PositionToCursor(p->start);
				lpKernelInfo->m_cCaretCoord = { (short)p->start.Sequence,(short)p->start.LineNumber };
				break;
			}
			case RD_DELETE:
			{
				lpKernelInfo->m_pCaretPixelPos = pCursor->PositionToCursor(p->end);
				lpKernelInfo->m_cCaretCoord = { (short)p->end.Sequence,(short)p->end.LineNumber };
				pCursor->Choose(p->start, p->end);		//�������ѡ����Ϣ
				lpKernelInfo->m_pStartPixelPos = pCursor->PositionToCursor_Before(pCursor->start);
				lpKernelInfo->m_pEndPixelPos = pCursor->PositionToCursor(pCursor->end);
				break;
			}
			case RD_REPLACE:
			{
				lpKernelInfo->m_pCaretPixelPos = pCursor->PositionToCursor(p->end);
				lpKernelInfo->m_cCaretCoord = { (short)p->end.Sequence,  (short)p->end.LineNumber };
				pCursor->Choose(p->start, p->end);
				lpKernelInfo->m_pStartPixelPos = pCursor->PositionToCursor_Before(pCursor->start);
				lpKernelInfo->m_pEndPixelPos = pCursor->PositionToCursor(pCursor->end);
				break;
			}
			default:break;
		}


		pRecord->pop();
		delete p;
		int New_LineNumbers = hText->Line_Number();
		lpKernelInfo->m_bLineBreak = (Old_LineNumbers == New_LineNumbers ? FALSE : TRUE);
		break;
	}
	case UM_PASTE:
	{
		
		Record* rd = new Record(RD_INSERT);
		std::string SText = wchTostring((TCHAR*)sParam);
		std::wstring WSText = StringToWString(SText);						//�����������

		Position start, end;												//��¼ճ�������һ���ַ�λ��
		if (pCursor->isChoose())
		{
			rd->ACT = RD_REPLACE;
			rd->Save_Delete_Data(hText, pCursor->start, pCursor->end);		//����ɾ��ǰ��Ϣ
			end = hText->Replace(pCursor->start, pCursor->end, WSText);		//end��¼��������һ���ַ�λ��
			rd->Set_Choose_Data(pCursor->start, end);
			pCursor->ResetChoose();
		}
		else
		{
			start = pCursor->CursorToPosition(x, y);
			Position s = { start.LineNumber,start.Sequence + 1 };			//���������ַ�λ��
			end = hText->Insert(start, WSText);								//�����ַ� start�õ���������һ���ַ���λ��

			rd->Set_Choose_Data(s, end);
		}

		pRecord->push(rd);
		/*�����ı���С ��ǰ���λ�� �߼�����*/
		lpKernelInfo->m_pTextPixelSize = { hText->Max_Line_Width(Width_EN),hText->Line_Number()*Height };
		lpKernelInfo->m_pCaretPixelPos = { pCursor->PositionToCursor(end) };
		lpKernelInfo->m_cCaretCoord = { (short)pCursor->Characters_before_Cursor(end.LineNumber,lpKernelInfo->m_pCaretPixelPos.x),(short)end.LineNumber };
		break;
	}
	case UM_FIND:
	{
		/*
		----------------------------------------------------------------------------------------------------------------------
		|ȫ�ĵ����� 1 ��ȫ�ĵ�����0	(16λ)|��ǰ��������� 1 ��ǰ�����ǰ���� 0 ��16λ��|���ִ�Сд 1 �����ִ�Сд 0��16λ��|
		---------------------------------------------------------------------------------------------------------------------
		*/

		/*���ò���ģʽ*/
		bool upper_lower = true;
		if (WORD(sParam) == 0)
			upper_lower = false;
		Position start = hText->First_Position();
		Position end = hText->End_Position();
		if ((WORD(HIDWORD(sParam)) == 0))
		{
			if (HIWORD(LODWORD(sParam)) > 0)
			{
				try 
				{
					start = pCursor->CursorToPosition_After(x, y);
				}
				catch (std::invalid_argument& e)
				{
					return UR_ERROR;					//���ı�ĩβ��ʼ���� �޷���ƥ��
				}
			}
			else
			{
				end = pCursor->CursorToPosition(x, y);
				if (end.Sequence == 0)
				{
					if (LineNumber == 1)				//���ı�ͷ֮ǰ���� �޷���ƥ��
						return UR_ERROR;
					else
					{
						end.LineNumber--;
						CLine* pLine = hText->GetLinePointer(end.LineNumber);
						end.Sequence = pLine->size();
					}
				}
			}
		}


		/*ȡ���������ַ���*/
		std::string SText = wchTostring((TCHAR*)fParam);
		std::wstring Str = StringToWString(SText);						


		if (hText->SeekStrings(Str, start, end, upper_lower))			//���ҳɹ�
		{
			POINT s = pCursor->PositionToCursor(end);
			(*(POINT*)fParam) = pCursor->PositionToCursor(end);
			pCursor->Choose(start, end);				
			lpKernelInfo->m_pCaretPixelPos = s;
			lpKernelInfo->m_cCaretCoord = { (short)end.Sequence ,(short)end.LineNumber };
			lpKernelInfo->m_pStartPixelPos = pCursor->PositionToCursor_Before(pCursor->start);
			lpKernelInfo->m_pEndPixelPos = pCursor->PositionToCursor(pCursor->end);
			break;
		}

		return UR_ERROR;												//δ�鵽
	}
	case UM_REPLACE:
	{
		/*ȡ��������ַ���*/
		std::string SText = wchTostring((TCHAR*)sParam);
		std::wstring Str = StringToWString(SText);

		/*ѡ�б��滻���ַ�����ʼ�յ�λ��*/
		Position start = pCursor->start;
		Position end = pCursor->end;

		/*���ó��� ���汻�滻�ַ���*/
		Record* rd = new Record(RD_REPLACE);
		rd->Save_Delete_Data(hText, start, end);

		end = hText->Replace(start, end, Str);

		pCursor->Choose(start, end);
		rd->Set_Choose_Data(start, end);
		pRecord->push(rd);
		/*���� �ı���С */
		lpKernelInfo->m_pTextPixelSize = { hText->Max_Line_Width(Width_EN),hText->Line_Number()*Height };
		/*���ø���*/
		lpKernelInfo->m_pStartPixelPos = pCursor->PositionToCursor_Before(start);
		lpKernelInfo->m_pEndPixelPos = pCursor->PositionToCursor(end);
		break;
	}
	case UM_ISSAVED:
	{
		if (hText->isSaved())
			return UR_SAVED;
		else
			return UR_NOTSAVED;
	}
	case UM_CHANGECHARSIZE:
	{
		Install::Width = x;
		Install::Height = y;
		pCursor->SetWidth(x);
		pCursor->SetHeight(y);
		break;
	}
	default: break;
	}
	return UR_SUCCESS;
}
//�����ı�����
HTEXT _stdcall CreateText(int iCharWidth, int iCharHeight, LPCWSTR lpszDefaultPath)
{
	CText* p = new CText;
	CText::Path = wchTostring((TCHAR*)lpszDefaultPath);
	p->NewFile();
	pTChar = new wchar_t[TEXT_SIZE];
	wmemset(pTChar, L' ', TEXT_SIZE);
	Install::Height = iCharHeight;
	Install::Width = iCharWidth;
	pCursor = Initialize_Cursor(p, iCharWidth, iCharHeight);			//���������
	pRecord = new std::stack<Record*>;									//���볷����
	return p;
}
//�����ı�����
BOOL _stdcall DestroyText(HTEXT &hText)
{
	if (hText != NULL)
		delete hText;
	hText = NULL;

	delete pRecord;
	pRecord = NULL;
	delete pCursor;
	pCursor = NULL;

	Free_Buffer(pTChar);
	Free_Buffer(pBuffer);
	return TRUE;
}

Cursor * Initialize_Cursor(CText * p, int Width, int Height)
{
	Cursor* pCursor = new Cursor(p, Width, Height);
	return pCursor;
}


//Ϊ���ݴ洢���뻺���� ��������Ϊ��������
void Alloc_Buffer(wchar_t *& p, size_t & Old_Size, size_t New_Size)
{
	if (p == NULL)
	{
		p = new wchar_t[New_Size];
		Old_Size = New_Size;
		return;
	}
	if (New_Size > Old_Size)				//��������Ҫ����
	{
		delete[]p;
		p = new wchar_t[New_Size];
		Old_Size = New_Size;
	}
	else
	{
		if (New_Size <= Old_Size / 4)		//���ٻ�������С �����ڴ�ռ�ù���
		{
			if (p != NULL)
				delete[] p;
			p = new wchar_t[New_Size];
			Old_Size = New_Size;
		}
	}

}




//�ͷŷ�������ݻ�����
void Free_Buffer(wchar_t *& p)
{
	if (p != NULL)
		delete[]p;
	p = NULL;
}

void Set_Height_Light(int LineNumber, Position ps, Position pe, short int & start, short int & end)
{
	Position s = pCursor->start;
	Position e = pCursor->end;
	int Len = pe.Sequence - ps.Sequence + 1;			//ѡ�γ���
	if (LineNumber<s.LineNumber || LineNumber>e.LineNumber)			//ѡ�β���ѡ�з�Χ
	{
		start = end = 0;
		return;
	}
	if (s.LineNumber == e.LineNumber)					//ѡ�з�Χֻ��һ��
	{
		start = max(0, s.Sequence - ps.Sequence);
		if (e.Sequence < ps.Sequence)
			end = 0;
		else if (e.Sequence > pe.Sequence)
			end = Len;
		else
			end = e.Sequence - ps.Sequence + 1;
		return;
	}
	if (LineNumber == s.LineNumber)
	{
		start = max(0, s.Sequence - ps.Sequence);
		end = Len;
	}
	else if (LineNumber == e.LineNumber)
	{
		start = 0;
		end = max(0, e.Sequence - ps.Sequence + 1);
	}
	else
	{
		start = 0;
		end = Len;
	}
	return;
}




