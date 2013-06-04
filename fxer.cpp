// fxdll.cpp : Defines the exported functions for the DLL application.
//

#include "fxer.h"

#include "commctrl.h"
#include "commdlg.h"

#include <stdio.h>

#include <map>
#include <vector>

#include <tchar.h>

//template <class T>
class vector : public std::vector<char>
{
public:
	// for write to file
	void push(const void* data,size_t len)
	{
		for(size_t i(0);i<len;++i)
		{
			push_back(((char*)data)[i]);
		}
	}
};

#ifdef _DEBUG
#define USE_DEBUG
#endif

#ifdef USE_DEBUG
#include <sstream>
#endif

enum fx_var_type
{
	b,
	c,
	uc,
	s,
	us,
	i,
	ui,
	l,
	ul,
	f,
	d,
	f2,
	f3,
	f4,
	m,
	num
};

struct MyStruct
{
	MyStruct():group(false),has_min(false),has_max(false){};
	ID3D10EffectVariable* v;
	HWND hWnd;
	const char* name;
	bool group;
	std::map<HWND,int> map;
	bool has_min;
	bool has_max;
	float min;
	float max;
	HWND hWnd_spin;
	union defValue
	{
		int i;
		float f;
		float f3[4];
		float f4[4];
	} dval;
};

int button_save_id   = 156;
int button_reset_id  = 157;
int button_color_id  = 158;
int button_bool_id   = 159;
int button_cancel_id = 160;
bool do_exit=false;
bool running=false;

TCHAR *g_fxname(TEXT("temp"));
HWND panel_buttons[3]={0};
HWND main_tab=0;
int w_buttons=0,h_buttons=0;

//template<class _Ty>
struct less2
	//: public std::binary_function<_Ty, _Ty, bool>
{	// functor for operator<
	bool operator()(const std::string/*_Ty*/& _Left, const std::string/*_Ty*/& _Right) const
	{	// apply operator< to operands
		return _Left.compare(_Right)<0;
	}
};

char* strtrim(char* str)
{
	int l = (int)strlen(str);
	bool f_dot(false);
	for (int i(0);i<l;++i)
	{
		if (str[i]=='.')
		{
			f_dot=true;
			break;
		}
	}

	if (!f_dot)
		return str;
	
	--l;
	for(;l>0;--l)
	{
		if (str[l]=='.'){ str[l]='\0'; break;}
		if ((str[l]=='0')||(str[l]=='.'))
		{
			str[l]='\0';			
		}
		else{break;}
	}
	return str;
}

//HWND edit;
std::map<HWND,MyStruct> map;
std::map<std::string,std::vector<ID3D10EffectVariable*>,less2/*,less/*<std::string>*/> var_map;
std::vector<HWND> groups;
int cc=0;
BOOL WINAPI myProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

VOID WINAPI DrawRectangle(HDC hdc, LONG x, LONG y, LONG width, LONG height, COLORREF bg) {
	
	HPEN pen = CreatePen(PS_NULL, 1, RGB(0, 0, 0));
	HBRUSH brush = NULL;
	HPEN prevPen = (HPEN)SelectObject(hdc, pen);
	HBRUSH prevBrush = NULL;
	
	brush = CreateSolidBrush(bg);
	prevBrush = (HBRUSH)SelectObject(hdc, brush);

	Rectangle(hdc, x, y, x+width+1, y+height+1);

	SelectObject(hdc, prevPen);
	SelectObject(hdc, prevBrush);

	DeleteObject(pen);
	DeleteObject(brush);
}

LRESULT CALLBACK groupProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message==WM_COMMAND){ 
	HWND parent_hwnd = GetAncestor((HWND)lParam,GA_PARENT);

	if (map.find(hwnd)!=map.end())
	{
		WORD tt = HIWORD(wParam);
		if (EN_CHANGE==tt)
		{
			char buffer[256];
			SendMessageA((HWND)lParam,WM_GETTEXT,256,(LPARAM)(buffer));
			float d=(float)atof(buffer);
			if (map[hwnd].group){
				map[hwnd].v->SetRawValue(&d,map[hwnd].map[(HWND)lParam]*4,4);
				InvalidateRect(hwnd,0,0);
			}
			else
			{
				HWND edit = (HWND)map[hwnd].map[(HWND)1];
				SendMessage(edit,TBM_SETPOS,TRUE,(long)d);
				map[hwnd].v->SetRawValue(&d,0,4);
			}			
		}
		if (LOWORD(wParam)==button_color_id)
		{
			CHOOSECOLOR cc = { 0 };
			cc.lStructSize = sizeof(cc);
			cc.hwndOwner = 0;
			COLORREF cust_colors[16] = { 0 };
			cc.lpCustColors = cust_colors;
			cc.Flags = CC_FULLOPEN | CC_RGBINIT;
			
			MyStruct& ms(map[hwnd]);
			float c3[3];
			ms.v->GetRawValue(&c3,0,4*3);
			COLORREF c=RGB(c3[0]*255,c3[1]*255,c3[2]*255);
			cc.rgbResult=c; 
			BOOL res = ChooseColor(&cc);
			if (res)
			{
				float col[3];
				col[0] = GetRValue(cc.rgbResult)/255.f;//256.f
				col[1] = GetGValue(cc.rgbResult)/255.f;
				col[2] = GetBValue(cc.rgbResult)/255.f;
				ms.v->SetRawValue(&col,0,4*3);

				int i(0);
				for (auto imap(ms.map.begin());imap!=ms.map.end();++imap)
				{
					if (imap->second>=3) continue;
					char buffer[256]="";
					sprintf((char *)&buffer,"%f",col[imap->second]);
					SendMessageA(imap->first,WM_SETTEXT,256,(LPARAM)strtrim(buffer));
				}
				RedrawWindow(hwnd,0,0,0);
				InvalidateRect(hwnd,0,0);
			}
		}
		if (LOWORD(wParam)==button_bool_id)
		{
			MyStruct& ms(map[hwnd]);
			HWND hwndCheck = ms.map.begin()->first;
			LRESULT res = SendMessage (hwndCheck, BM_GETCHECK, 0, 0);

			ms.v->SetRawValue(&res,0,4);			
		}
	}
	
	}// if (map.find(hwnd)!=map.end())

	switch (message)
	{
	case WM_HSCROLL:
		{
			switch(LOWORD(wParam))
			{
				
			case TB_LINEUP:
			case TB_LINEDOWN:
			case TB_PAGEUP:
			case TB_PAGEDOWN:
			case TB_TOP:
			case TB_BOTTOM:
			case TB_THUMBPOSITION:
				{
					MyStruct& ms(map[hwnd]);
					int pos=(int)SendMessage((HWND)lParam,TBM_GETPOS,0,0);
					float pos_tbm = (ms.max-ms.min)*pos/100.f+ms.min;
					char buffer[256]="";
					sprintf((char *)&buffer,"%f",(float)pos_tbm);
					HWND edit = (HWND)ms.map[(HWND)0];
					SendMessageA(edit,WM_SETTEXT,256,(LPARAM)strtrim(buffer));
					map[hwnd].v->SetRawValue(&pos_tbm,0,4);
					HWND edit2 = (HWND)ms.map[(HWND)1];
					SendMessage(edit2,TBM_SETPOS,TRUE,(long)pos);
					return true;
				}break;
			}
		}break;		
		case WM_DRAWITEM:{
			DRAWITEMSTRUCT* pdis = (DRAWITEMSTRUCT*) lParam;
			RECT r = pdis->rcItem;
			float c[3];
			map[hwnd].v->GetRawValue(&c,0,4*3);
			DrawRectangle(pdis->hDC,r.top,r.left,r.right,r.bottom,RGB(255*c[0], 255*c[1], 255*c[2]));
			return true;
		}break;
		case WM_PAINT:
			{

				PAINTSTRUCT ps;
				HDC hDC=BeginPaint(hwnd,&ps);

				EndPaint(hwnd,&ps);
			}
			break;	
		case WM_NOTIFY:{
			float iCurveScale =1;
			NM_UPDOWN *ud=(NM_UPDOWN *)lParam;
			if(ud->hdr.code==UDN_DELTAPOS) {
				HWND edit = (HWND)SendMessage(ud->hdr.hwndFrom,UDM_GETBUDDY,0,0);
				char buffer[256];
				SendMessageA(edit,WM_GETTEXT,256,(LPARAM)(buffer));
				float d=(float)atof(buffer);
				
				if(ud->iDelta>0){
					d+=0.01f;
				}
				else
					d-=0.01f;

				sprintf((char *)&buffer,"%f",d);				
				SendMessageA(edit,WM_SETTEXT,256,(LPARAM)strtrim(buffer));
			}			
		}break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);;
}

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#endif

void reset_vars()
{
	if (!map.size())
		return ;
	for (auto it(map.begin());it!=map.end();++it)
	{
		ID3D10EffectType* t = it->second.v->GetType();
		D3D10_EFFECT_TYPE_DESC td;
		t->GetDesc(&td);
		if (strcmp(td.TypeName,"float")==0)
		{
			if (it->second.group)
			{
				char buffer[256]="";
				sprintf((char *)&buffer,"%f",it->second.dval.f);
				HWND edit = (HWND)it->second.map[(HWND)0];
				SendMessageA(edit,WM_SETTEXT,256,(LPARAM)strtrim(buffer));

				HWND spin = (HWND)it->second.map[(HWND)1];
				SendMessage(spin,TBM_SETPOS,TRUE,(long)it->second.dval.f);
				it->second.v->SetRawValue(&it->second.dval.f,0,4);
			}
			else
			{
				char buffer[256]="";
				sprintf((char *)&buffer,"%f",it->second.dval.f);
				SendMessageA(it->first,WM_SETTEXT,256,(LPARAM)strtrim(buffer));

				it->second.v->SetRawValue(&it->second.dval.f,0,4);
			}
		}
		if (strcmp(td.TypeName,"float4")==0)
		{
			for (auto imap(it->second.map.begin());imap!=it->second.map.end();++imap)
			{
				char buffer[256]="";
				sprintf((char *)&buffer,"%f",it->second.dval.f4[imap->second]);
				SendMessageA(imap->first,WM_SETTEXT,256,(LPARAM)strtrim(buffer));
			}
			it->second.v->SetRawValue(&it->second.dval.f4,0,4*4);
		}
		if (strcmp(td.TypeName,"float3")==0)
		{
			for (auto imap(it->second.map.begin());imap!=it->second.map.end();++imap)
			{
				char buffer[256]="";
				sprintf((char *)&buffer,"%f",it->second.dval.f3[imap->second]);
				SendMessageA(imap->first,WM_SETTEXT,256,(LPARAM)strtrim(buffer));
			}
			it->second.v->SetRawValue(&it->second.dval.f3,0,4*3);
		}
		if (strcmp(td.TypeName,"bool")==0)
		{
			HWND ch = it->second.map.begin()->first;
			BOOL check = it->second.dval.i;

			SendMessage(ch,BM_SETCHECK,check,0);
			//SetWindowTextA(ch,check?"true":"false");

			it->second.v->SetRawValue(&check,0,4);
		}
	}
}

BOOL WINAPI myProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
	static int nHorizDifference=0,nVertDifference=100;  
	static int nHorizPosition=0,nVertPosition=100;

	static bool middle_button(false);
	static int xPos(0);
	static int yPos(0); 

	int vInc=0;

	if(message == WM_CLOSE) {
		CloseWindow(hwnd);
		DestroyWindow(hwnd);
		do_exit=true;
		running=false;
		free(g_fxname);
		g_fxname=0;		
	}

	//check if button was clicked
	if(message==WM_COMMAND){ 
		if (LOWORD(wParam)==button_reset_id)
		{
			reset_vars();
		}
		if (LOWORD(wParam)==button_cancel_id)
		{
			reset_vars();
			SendMessage(hwnd,WM_CLOSE,0,0);
		}
		if (LOWORD(wParam)==button_save_id)
		{
			if (!map.size())
				return false;

			vector data;

			for (auto it(map.begin());it!=map.end();++it)
			{
				ID3D10EffectType* t = it->second.v->GetType();
				D3D10_EFFECT_TYPE_DESC td;
				t->GetDesc(&td);
				fx_var_type vt;
				if (strcmp(td.TypeName,"float")==0)
				{
					int len = (int)strlen(it->second.name);
					//w=fwrite(&len,1,1,f);// int to byte!!
					data.push(&len,1);
					//w=fwrite(it->second.name,len,1,f);
					data.push(it->second.name,len);

					vt=fx_var_type::f;
					//w=fwrite(&vt,1,1,f);
					data.push(&vt,1);

					ID3D10EffectScalarVariable* value = it->second.v->AsScalar();
					float fxval = 0;
					value->GetFloat(&fxval);
					//w=fwrite(&fxval,4,1,f);
					data.push(&fxval,4);
				}
				if (strcmp(td.TypeName,"float4")==0)
				{
					int len = (int)strlen(it->second.name);
					//w=fwrite(&len,1,1,f);// int to byte!!
					data.push(&len,1);
					//w=fwrite(it->second.name,len,1,f);
					data.push(it->second.name,len);

					vt=fx_var_type::f4;
					//w=fwrite(&vt,1,1,f);
					data.push(&vt,1);

					ID3D10EffectVectorVariable* value = it->second.v->AsVector();
					float fxval[4]={0};
					value->GetFloatVector((float*)&fxval);
					//w=fwrite(&fxval,4*4,1,f);
					data.push(&fxval,4*4);
				}
				if (strcmp(td.TypeName,"float3")==0)
				{
					int len = (int)strlen(it->second.name);
					//w=fwrite(&len,1,1,f);// int to byte!!
					data.push(&len,1);
					//w=fwrite(it->second.name,len,1,f);
					data.push(it->second.name,len);

					vt=fx_var_type::f3;
					//w=fwrite(&vt,1,1,f);
					data.push(&vt,1);

					ID3D10EffectVectorVariable* value = it->second.v->AsVector();
					float fxval[3]={0};
					value->GetFloatVector((float*)&fxval);
					//w=fwrite(&fxval,4*3,1,f);
					data.push(&fxval,4*3);
				}
				if (strcmp(td.TypeName,"bool")==0)
				{
					int len = (int)strlen(it->second.name);
					//w=fwrite(&len,1,1,f);// int to byte!!
					data.push(&len,1);
					//w=fwrite(it->second.name,len,1,f);
					data.push(it->second.name,len);

					vt=fx_var_type::b;
					//w=fwrite(&vt,1,1,f);
					data.push(&vt,1);

					ID3D10EffectScalarVariable* value = it->second.v->AsScalar();
					BOOL fxval = 0;
					value->GetBool(&fxval);
					//w=fwrite(&fxval,4,1,f);
					data.push(&fxval,1);
				}
			}

			FILE* f = 0;//fopen(g_fxname,"rb+");
			
			/*if (saveType==_3da){
				f=_tfopen(g_fxname,TEXT("rb+"));
				fseek(f,sizeof(EXPORT_SCENEHEADER),SEEK_SET);
			}
			else
			{*/
				f=_tfopen(g_fxname,TEXT("wb"));
				if (!f)
				{
					HRESULT hr = GetLastError();
					//GetErrorInfo()
					TCHAR strhr[80]=L"\0";
					wsprintf(strhr,L"%d",hr);
					MessageBox(0,g_fxname,strhr,0);
					return 0;
				}
			//}

			int size=(int)data.size();
			fwrite(&size,4,1,f);

			fwrite(&data[0],size,1,f);

			fclose(f);

			//MessageBox(hwnd,TEXT("fx saved.."),TEXT("fx"),0);
			//CloseWindow(hwnd);
			SendMessage(hwnd,WM_CLOSE,0,0);
		}

		//HWND parent_hwnd = GetAncestor((HWND)LOWORD(wParam),GA_PARENT);

		if (map.find((HWND)lParam)!=map.end())
		{
			WORD tt = HIWORD(wParam);
			if (EN_CHANGE==tt)
			{
				char buffer[256];
				SendMessageA((HWND)lParam,WM_GETTEXT,256,(LPARAM)(buffer));
				float d=(float)atof(buffer);
				map[(HWND)lParam].v->SetRawValue(&d,0,4);
			}
		}
	}

	switch (message)
	{
		case WM_VSCROLL:
		{
			//
		}break;		
		case WM_MOUSEMOVE:
		{
			if (0/*middle_button*/){
				int x = GET_X_LPARAM(lParam); 
				int y = GET_Y_LPARAM(lParam);

				int inc = min(max(yPos-y,-1),1);
#ifdef USE_DEBUG
				std::stringstream str;
				str<<yPos-y<<" inc "<<inc<<'\n';
				OutputDebugStringA(str.str().c_str());
#endif
				ScrollWindow(hwnd,0,inc/*yPos-y*//*vInc*//*nVertPosition*/,0,0);

				return 0;
			}
		}break;
		case WM_MBUTTONDOWN:
		{
			/*int*/ xPos = GET_X_LPARAM(lParam); 
			/*int*/ yPos = GET_Y_LPARAM(lParam);
			middle_button = true;
		}break;
		case WM_MBUTTONUP:{
			int xPos=0;
			int yPos=0;
			middle_button = false;
		}break;
		case WM_SIZE:{
			int w = GET_X_LPARAM(lParam); 
			int h = GET_Y_LPARAM(lParam);
			w_buttons=w;
			h_buttons=h;
			static int mt_off = 10;
			SetWindowPos(panel_buttons[0],HWND_BOTTOM,w-320,h-25,100,25,0);
			SetWindowPos(panel_buttons[1],HWND_BOTTOM,w-210,h-25,100,25,0);
			SetWindowPos(panel_buttons[2],HWND_BOTTOM,w-100,h-25,100,25,0);
			SetWindowPos(main_tab,0,0+mt_off,0+mt_off,w-mt_off,h-25-mt_off,0);
			for (auto it(groups.begin());it!=groups.end();++it)
				SetWindowPos(*it,0,0+mt_off,30+mt_off,w-mt_off,h-25-mt_off,0);
		}break;	
		case WM_PAINT:
			{

				PAINTSTRUCT ps;
				HDC hDC=BeginPaint(hwnd,&ps);

				EndPaint(hwnd,&ps);
			}
			break;
		case WM_NOTIFY:	{	/* Process a tab change		*/
			LPNMHDR nmptr=(LPNMHDR)lParam;
			if(nmptr->code == TCN_SELCHANGE)
			{
				for (auto it(groups.begin());it!=groups.end();++it)
					ShowWindow(*it,SW_HIDE);
				int tabnumber = TabCtrl_GetCurSel((HWND)nmptr->hwndFrom);
				TCITEM tbi={0};
				tbi.mask = /*TCIF_TEXT|*/TCIF_PARAM;
				TabCtrl_GetItem((HWND)nmptr->hwndFrom,tabnumber,&tbi);
				HWND but = (HWND)tbi.lParam;
				ShowWindow(but,SW_SHOW);				
			}
			
		}break;
		//default:
			//DefWindowProcW(hwnd, message, wParam, lParam);
	}//mes
	return false; 
}

int fnfxdll_edit_withname(ID3D10Effect* effect/*void*/,const TCHAR* fxname, HWND parent/*,SaveType st*/)
{
	//g_fxname = fxname;
	g_fxname = (TCHAR*)malloc(sizeof(TCHAR)*256);
	_tcscpy(g_fxname,fxname);
	//saveType=st;
	int ret = fnfxdll_edit(effect,parent);
	return ret;
}
HWND myDialog=0;
int fnfxdll_edit(ID3D10Effect* e/*void*/, HWND parent)
{	
	if (running){
		return 1;
	}
	
	running=true;

	WNDCLASSW rwc = {0};
	rwc.lpszClassName = L"BluePanelClass";
	rwc.lpfnWndProc   = groupProc;
	rwc.hCursor       = LoadCursor(0, IDC_ARROW);
	RegisterClassW(&rwc);

	do_exit = false;

	int dx(400),dy(100),dw(640),dh(480);
	
	MSG msg;
	myDialog = CreateWindowEx(
		WS_EX_DLGMODALFRAME|WS_EX_CLIENTEDGE|WS_EX_TOOLWINDOW /*| WS_EX_TOPMOST*/,WC_DIALOG,TEXT("fx_edit"),WS_OVERLAPPEDWINDOW | WS_VISIBLE/*| WS_HSCROLL | WS_VSCROLL*/,
		dx,dy,dw,dh/*400,100,200,200*/, parent,NULL,NULL,NULL
		);

	main_tab = CreateWindow(WC_TABCONTROL,0,WS_VISIBLE|WS_CHILD,20,20,320,240,myDialog,0,0,0);
	
	DWORD err = GetLastError();

	D3D10_EFFECT_DESC desc;
	e->GetDesc(&desc);
	var_map.clear();
	for (UINT i=0;i<desc.GlobalVariables;++i)
	{
		ID3D10EffectVariable* v =  e->GetVariableByIndex(i);
		const char* var_name=0;
		const char* group_name=0;
		ID3D10EffectVariable* pAnn_g = v->GetAnnotationByName("group");
		pAnn_g->AsString()->GetString(&group_name);//deffirent addres names!!
		std::string gr;
		if(group_name)
			gr=group_name;
		ID3D10EffectVariable* pAnn_n = v->GetAnnotationByName("name");
		pAnn_n->AsString()->GetString(&var_name);
		if (var_name){
			var_map[gr/*group_name*/].push_back(v);//=v;			
		}
	}
	map.clear();
	int counter_group=0;
	for (auto it(var_map.rbegin()); it!=var_map.rend(); ++it)
	{		
		HWND gr = CreateWindow(TEXT("BluePanelClass"), 0, /*WS_VISIBLE|*/WS_CHILD/*|WS_BORDER*/,0,30,110,80,main_tab,0,0,0);
		groups.push_back(gr);
		TCITEMA tbi={0};	
		tbi.mask = TCIF_TEXT|TCIF_PARAM;
		tbi.lParam = (LPARAM)gr;
		tbi.pszText = (char*)it->first.c_str();
		//TabCtrl_InsertItem(main_tab,counter_group++,&tbi);
		(int)SNDMSG((main_tab), TCM_INSERTITEMA, (WPARAM)(int)(counter_group++), (LPARAM)(const TC_ITEMA *)(&tbi));

		int count_vars(0);
		for (UINT m=0;m<it->second.size();++m)
		{
			ID3D10EffectVariable* v =  it->second[m];
			bool has_color=false;
			D3D10_EFFECT_VARIABLE_DESC vd;
			v->GetDesc(&vd);
			int x_offset=198;//149;
			int edit_width = 100;//56;//76;//80;
			int edit_height = 25;
			const char* var_name=0;
			ID3D10EffectVariable* pAnn = v->GetAnnotationByName("name");
			ID3D10EffectVariable* acol = v->GetAnnotationByName("color");
			D3D10_EFFECT_VARIABLE_DESC acoldesc;
			if (acol->GetDesc(&acoldesc)==0) has_color=true;
			pAnn->AsString()->GetString(&var_name);
			ID3D10EffectType* t = v->GetType();
			D3D10_EFFECT_TYPE_DESC td;
			t->GetDesc(&td);
			if (strcmp(td.TypeName,"float")==0)
			{				
				int height = 10+count_vars++*edit_height;
				
				CreateWindowExA(0,"STATIC",var_name?var_name:vd.Name,WS_VISIBLE|WS_CHILD/*|WS_BORDER*/,10,height,x_offset,edit_height,gr,0,0,0);
				
				ID3D10EffectScalarVariable* value = v->AsScalar();
				float fxval = 0;
				value->GetFloat(&fxval);
				char str[80]={0};
				//_gcvt(fxval,10,(char *)&str); !!!! last char??
				sprintf((char *)&str,"%f",fxval);

				// for if min max!!
				MyStruct fx_h;
				fx_h.v = v;
				fx_h.name = vd.Name;
				fx_h.dval.f = fxval;

				// min max!!
				ID3D10EffectVariable* amin = v->GetAnnotationByName("min");
				D3D10_EFFECT_VARIABLE_DESC amin_desc;
				HRESULT hr = amin->GetDesc(&amin_desc);
				if (hr==0)
				{
					float val_min;
					amin->GetRawValue(&val_min,0,4);				
					fx_h.has_min=true;
					fx_h.min=val_min;
				}
				ID3D10EffectVariable* amax = v->GetAnnotationByName("max");
				D3D10_EFFECT_VARIABLE_DESC amax_desc;
				hr = amax->GetDesc(&amax_desc);
				if (hr==0)
				{
					float val_max;
					amax->GetRawValue(&val_max,0,4);				
					fx_h.has_max=true;
					fx_h.max=val_max;
				}

				//HWND edit;
				if (!fx_h.has_max||!fx_h.has_min)
				{
					HWND g = CreateWindowExA(0,"BluePanelClass", 0, WS_VISIBLE | WS_CHILD  /*| WS_BORDER*/,
						x_offset,height,edit_width,edit_height,gr,(HMENU)0,NULL,NULL);

					HWND edit1 = CreateWindowExA(WS_EX_CLIENTEDGE,"EDIT",strtrim(str),WS_CHILD|WS_VISIBLE/*|WS_BORDER*/,0,0,edit_width,edit_height,g,0,0,0);
					HWND edit2 = CreateUpDownControl(UDS_NOTHOUSANDS|UDS_ARROWKEYS|UDS_ALIGNRIGHT|WS_CHILD|WS_VISIBLE,0,0,0,0,g,0,0,edit1,10,0,0);
					fx_h.hWnd = g;
					map[g]=fx_h;
				}
				else
				{
					HWND g = CreateWindowExA(0,"BluePanelClass", 0, WS_VISIBLE | WS_CHILD  /*| WS_BORDER*/,
						x_offset,height,edit_width*2,edit_height,gr,(HMENU)0,NULL,NULL);

					HWND edit1 = CreateWindowExA(WS_EX_CLIENTEDGE,"EDIT",strtrim(str),WS_CHILD|WS_VISIBLE,0,0,edit_width,edit_height,g,(HMENU)0,0,0);
					HWND edit2 = CreateWindowEx(0/*WS_EX_CLIENTEDGE*/,TRACKBAR_CLASS,0,WS_VISIBLE|WS_CHILD,edit_width,0,edit_width,edit_height,g,0,0,0);
					float pos_tbm = ((fx_h.dval.f-fx_h.min)*100.f)/(fx_h.max-fx_h.min);				
					SendMessage(edit2,TBM_SETRANGE,TRUE,MAKELONG(0,100));
					SendMessage(edit2,TBM_SETPOS,TRUE,(long)pos_tbm);
					fx_h.hWnd = g;
					fx_h.map[(HWND)0/*edit1*/] = (int)edit1;//0;
					fx_h.map[(HWND)1/*edit2*/] = (int)edit2;//1;
					//fx_h.group=true;
					map[g]=fx_h;
				}

				count_vars++;
			}
			if (strcmp(td.TypeName,"float3")==0)
			{
				int height = 10+count_vars++*edit_height;
				int height_text = 10+count_vars++*edit_height;
				int height_e3 = 10+count_vars++*edit_height;
				
				CreateWindowExA(0,"STATIC",var_name?var_name:vd.Name,SS_SIMPLE|WS_VISIBLE|WS_CHILD/*|WS_BORDER*/,10,height_text,x_offset,edit_height,gr,0,0,0);						

				ID3D10EffectVectorVariable* value = v->AsVector();
				float fxval[3];
				value->GetFloatVector((float*)&fxval);

				char str1[80]={0};// '\0' ??
				sprintf((char *)&str1,"%f",fxval[0]);
				char str2[80]={0};
				sprintf((char *)&str2,"%f",fxval[1]);
				char str3[80]={0};
				sprintf((char *)&str3,"%f",fxval[2]);
				
				HWND g = CreateWindowExA(0,"BluePanelClass", 0, WS_VISIBLE | WS_CHILD /*| WS_BORDER*/,
					x_offset,height,/*x_offset+*/edit_width+(has_color?edit_width:0),edit_height*3/*height+edit_height*(3)*/,gr,0,0,0);

				HWND edit1 = CreateWindowExA(WS_EX_CLIENTEDGE,"EDIT",strtrim(str1),WS_CHILD|WS_VISIBLE,0,            0,edit_width,edit_height,g,(HMENU)0,0,0);
				HWND edit2 = CreateWindowExA(WS_EX_CLIENTEDGE,"EDIT",strtrim(str2),WS_CHILD|WS_VISIBLE,0,edit_height*1,edit_width,edit_height,g,(HMENU)1,0,0);
				HWND edit3 = CreateWindowExA(WS_EX_CLIENTEDGE,"EDIT",strtrim(str3),WS_CHILD|WS_VISIBLE,0,edit_height*2,edit_width,edit_height,g,(HMENU)2,0,0);

				CreateUpDownControl(UDS_NOTHOUSANDS|UDS_ARROWKEYS|UDS_ALIGNRIGHT|WS_CHILD|WS_VISIBLE,0,0,0,0,g,0,0,edit1,10,0,0);
				CreateUpDownControl(UDS_NOTHOUSANDS|UDS_ARROWKEYS|UDS_ALIGNRIGHT|WS_CHILD|WS_VISIBLE,0,0,0,0,g,0,0,edit2,10,0,0);
				CreateUpDownControl(UDS_NOTHOUSANDS|UDS_ARROWKEYS|UDS_ALIGNRIGHT|WS_CHILD|WS_VISIBLE,0,0,0,0,g,0,0,edit3,10,0,0);

				HWND editcolor;
				if (has_color) 
					editcolor = CreateWindowExA(WS_EX_CLIENTEDGE,"BUTTON","c",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,edit_width,0/*height*/,edit_width,edit_height*3,g,(HMENU)button_color_id,0,0);
							
				MyStruct fx_h;
				fx_h.group=true;
				fx_h.v = v;
				fx_h.hWnd = g;
				fx_h.name = vd.Name;
				fx_h.map[edit1]=0;
				fx_h.map[edit2]=1;
				fx_h.map[edit3]=2;
				if(has_color)fx_h.map[editcolor]=3;
				memcpy(fx_h.dval.f3,fxval,sizeof(float[3]));
				map[g]=fx_h;

				count_vars++;
			}
			if (strcmp(td.TypeName,"float4")==0)
			{
				//int i = count_vars++;
				int height = 10+count_vars++*edit_height;
				int height_text = 10+count_vars++*edit_height;
				int height_e3 = 10+count_vars++*edit_height;
				int height_e4 = 10+count_vars++*edit_height;
				
				CreateWindowExA(0,"STATIC",var_name?var_name:vd.Name,SS_SIMPLE|WS_VISIBLE|WS_CHILD/*|WS_BORDER*/,10,height_text+edit_height/2,x_offset,edit_height,gr,0,0,0);						

				ID3D10EffectVectorVariable* value = v->AsVector();
				float fxval[4];
				value->GetFloatVector((float*)&fxval);

				char str1[80]={0};// '\0' ??
				sprintf((char *)&str1,"%f",fxval[0]);
				char str2[80]={0};
				sprintf((char *)&str2,"%f",fxval[1]);
				char str3[80]={0};
				sprintf((char *)&str3,"%f",fxval[2]);
				char str4[80]={0};
				sprintf((char *)&str4,"%f",fxval[3]);
				
				HWND g = CreateWindowExA(0,"BluePanelClass", 0, WS_VISIBLE | WS_CHILD /*| WS_BORDER*/,
					x_offset,height,/*x_offset+*/edit_width+(has_color?edit_width:0),edit_height*4,gr,0,0,0);

				HWND edit1 = CreateWindowExA(WS_EX_CLIENTEDGE,"EDIT",strtrim(str1),WS_CHILD|WS_VISIBLE,0,            0,edit_width,edit_height,g,(HMENU)0,0,0);
				HWND edit2 = CreateWindowExA(WS_EX_CLIENTEDGE,"EDIT",strtrim(str2),WS_CHILD|WS_VISIBLE,0,edit_height*1,edit_width,edit_height,g,(HMENU)1,0,0);
				HWND edit3 = CreateWindowExA(WS_EX_CLIENTEDGE,"EDIT",strtrim(str3),WS_CHILD|WS_VISIBLE,0,edit_height*2,edit_width,edit_height,g,(HMENU)2,0,0);
				HWND edit4 = CreateWindowExA(WS_EX_CLIENTEDGE,"EDIT",strtrim(str4),WS_CHILD|WS_VISIBLE,0,edit_height*3,edit_width,edit_height,g,(HMENU)3,0,0);

				CreateUpDownControl(UDS_NOTHOUSANDS|UDS_ARROWKEYS|UDS_ALIGNRIGHT|WS_CHILD|WS_VISIBLE,0,0,0,0,g,0,0,edit1,10,0,0);
				CreateUpDownControl(UDS_NOTHOUSANDS|UDS_ARROWKEYS|UDS_ALIGNRIGHT|WS_CHILD|WS_VISIBLE,0,0,0,0,g,0,0,edit2,10,0,0);
				CreateUpDownControl(UDS_NOTHOUSANDS|UDS_ARROWKEYS|UDS_ALIGNRIGHT|WS_CHILD|WS_VISIBLE,0,0,0,0,g,0,0,edit3,10,0,0);
				CreateUpDownControl(UDS_NOTHOUSANDS|UDS_ARROWKEYS|UDS_ALIGNRIGHT|WS_CHILD|WS_VISIBLE,0,0,0,0,g,0,0,edit4,10,0,0);

				HWND editcolor;
				if (has_color) 
					editcolor = CreateWindowExA(WS_EX_CLIENTEDGE,"BUTTON","c",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,edit_width,0/*height*/,edit_width,edit_height*4,g,(HMENU)button_color_id,0,0);

				MyStruct fx_h;
				fx_h.group=true;
				fx_h.v = v;
				fx_h.hWnd = g;
				fx_h.name = vd.Name;
				fx_h.map[edit1]=0;
				fx_h.map[edit2]=1;
				fx_h.map[edit3]=2;
				fx_h.map[edit4]=3;
				if(has_color)fx_h.map[editcolor]=4;
				memcpy(fx_h.dval.f4,fxval,sizeof(float[4]));
				map[g]=fx_h;

				count_vars++;
			}
			if (strcmp(td.TypeName,"bool")==0)
			{
				int height = 10+count_vars++*edit_height;
				CreateWindowExA(0,"STATIC",var_name?var_name:vd.Name,WS_VISIBLE|WS_CHILD,10,height,x_offset,edit_height,gr,0,0,0);

				ID3D10EffectScalarVariable* value = v->AsScalar();
				BOOL fxval;
				value->GetBool(&fxval);

				HWND g = CreateWindowExA(0,"BluePanelClass", 0, WS_VISIBLE | WS_CHILD,
					x_offset,height,edit_width,edit_height,gr,(HMENU)0,NULL,NULL);
				
				HWND edit1 = CreateWindowExA(0/*WS_EX_CLIENTEDGE*/,"button",""/*fxval?"true":"false"*/,WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX/*|WS_BORDER*/,0,0,edit_width,edit_height,g,(HMENU)button_bool_id,0,0);

				if (fxval)
					SendMessage(edit1,BM_SETCHECK,BST_CHECKED,0);

				MyStruct fx_h;
				fx_h.v = v;
				fx_h.name = vd.Name;
				fx_h.dval.i = fxval;
				fx_h.hWnd = g;
				fx_h.map[edit1]=0;
				map[g]=fx_h;
			}
		}
	}	

	//height = winRect.y+10;

	w_buttons=110;
	h_buttons=25;
	//create button
	panel_buttons[1] = CreateWindowEx(
		0,TEXT("BUTTON"),TEXT("ok"),BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD,
		10,320/*height*/+20,100,25,/*panel_buttons*/myDialog,(HMENU)button_save_id,NULL,NULL
		);
	panel_buttons[2] = CreateWindowExA(
		0,"BUTTON","reset",BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD,
		110+10,320/*height*/+20,100,25,/*panel_buttons*/myDialog,(HMENU)button_reset_id,NULL,NULL
		);
	panel_buttons[0] = CreateWindowExA(
		0,"BUTTON","cancel",BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD,
		220+10,320/*height*/+20,100,25,/*panel_buttons*/myDialog,(HMENU)button_cancel_id,NULL,NULL
		);

	//SetWindowLong(myDialog, DWL_DLGPROC, (LONG)myProc); only 32!!
	SetWindowLongPtr(myDialog, DWLP_DLGPROC, (LONG_PTR)myProc);

	if ((dh-40)<320/*height*/)
	{
		dh=320/*height*/+20+40+25/*40+50*/;
		if (dh>800)
			dh=800;
		SetWindowPos(myDialog,0,dx,dy,dw+90,dh,0);
	}
	SetWindowPos(myDialog,0,dx,dy,640,480,0);
	//InvalidateRect(myDialog,0,true);

	SendMessage(myDialog,WM_SIZE,0,MAKELONG(620,442));

	while(GetMessage(&msg,NULL,0,0)&&do_exit)  {
		TranslateMessage(&msg);
		DispatchMessage(&msg); 
	} 

	TabCtrl_SetCurSel(main_tab,0/*counter_group-1*/);
	if (groups.size()>0){
		ShowWindow(groups[0/*counter_group-1*/],SW_SHOW|SW_SHOWNORMAL);
		NMHDR nmptr = {0};
		nmptr.code = TCN_SELCHANGE;
		nmptr.hwndFrom = main_tab;
		SendMessage(main_tab,WM_NOTIFY,0,(LPARAM)&nmptr);
	}

	return 1;
}

int fnfxdll_setvars(ID3D10Effect* effect,const TCHAR* f_name/*,SaveType st*/)
{
	FILE* file = _tfopen(f_name,TEXT("rb"));
	if (!file)
		return 0;

	//saveType=st;

	int size_header=0;
	/*if (saveType==_3da){
		EXPORT_SCENEHEADER H;
		size_header=sizeof(H);
		fread(&H,size_header,1,file);		
	}*/

	int data_size;// = sizeof(H)+H.iFXSettingsSize;
	fread(&data_size,4,1,file);
	if (data_size<=0)
		return 0;
	data_size+=size_header;

	while(!feof(file)&&(ftell(file)<data_size))
	{
		UCHAR slen;
		fread(&slen,1,1,file);
		char var_name[256];
		fread(&var_name,slen,1,file);
		var_name[slen]='\0';

		fx_var_type vt(fx_var_type::num);
		fread(&slen,1,1,file);
		vt=(fx_var_type)slen;
		switch (vt)
		{
		case b:{
			BOOL f;
			fread(&f,1,1,file);
			ID3D10EffectVariable* v = effect->GetVariableByName(var_name);
			v->SetRawValue(&f,0,1);
		}break;
		case c:
			break;
		case uc:
			break;
		case s:
			break;
		case us:
			break;
		case i:
			break;
		case ui:
			break;
		case l:
			break;
		case ul:
			break;
		case f:{
			float f;
			fread(&f,4,1,file);
			ID3D10EffectVariable* v = effect->GetVariableByName(var_name);
			v->SetRawValue(&f,0,4);
		}break;
		case d:
			break;
		case f3:{
			float f[3];
			fread(&f,4*3,1,file);
			ID3D10EffectVariable* v = effect->GetVariableByName(var_name);
			v->SetRawValue(&f,0,4*3);
		}break;
		case f4:{
			float f[4];
			fread(&f,4*4,1,file);
			ID3D10EffectVariable* v = effect->GetVariableByName(var_name);
			v->SetRawValue(&f,0,4*4);
		}break;
		case m:
			break;
		default:
			break;
		}
	}

	fclose(file);

	return 1;
}
