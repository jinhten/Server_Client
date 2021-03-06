// include zbNet header
#include "zbNet.h"
#include <vector>
#include <limits>
#include <stdio.h>
#include <bit>

//////////////////////////////////////////////////////////////////
// window class 
class zibLinux
{
public:
    zbNet     _net;

    struct FileInfos
    {
        long long date = 0;
        short type = 0;
        short state = 0;
        int dummy = 0;
        string name = "";
    };
    ///////////////////////////////
    // interface functions

    // create
/**
@brief  zibNet 의 초기화 작업 담당
*/
    void init(const zbNet::PathSet& pathSet, const string& deviceID, const string& deviceName)
    {
        _net.SetNksAddr(kmAddr4(52, 231, 35, 166, DEFAULT_PORT));
        //_net.SetNksAddr(kmAddr4(10, 114, 75, 52, DEFAULT_PORT));
        CreateChild(pathSet, deviceID, deviceName);
    };

/**
@brief   LAN Network 을 통한 집서버 연결

@return  0 : 현재 단말에 등록된 zibServer가 없는 경우. (LAN에 기기가 없을 때)
             이미 연결된 동일한 mac address가 있는 경우.
             mac address로 Connect에 실패한 경우.
         1 : 등록된 mac address로 Connect에 성공한 경우.
*/
    int connectToLan()
    {
        if (_net._users.N1() > 0) return -1;

        // auto connection
        int ret = ConnectBroadcast();
        if (ret == 0) return -1;

        return ret;
    }

/**
@brief   집서버 연결
         1. 마지막으로 연결한 집서버 ip/port로 연결시도
         2. 실패하면 Broad cast로 연결시도
         3. 실패하면 RTC로 pkey와 mapping되는 집서버 ip/port를 수신하여 연결시도

@return  0 : 집서버와 연결된 적이 없는 경우.
         1 : 집서버 Connect에 성공한 경우.
*/
    int connectWithZibSvr()
    {
        return _net.Connect(0);
    }

/**
@brief   등록된 User가 아닌 현재 zibServer와 연결 여부를 확인한다.

@return  0 : 연결 X
         1 : 연결 O
*/
    int checkConnect()
    {
        if (_net.GetIds().N() < 1) return 0;
        return 1;
    }

/**
@brief  WAN Network 을 통한 집서버 연결

@return  0 : 현재 단말에 등록된 zibServer가 없는 경우. (한번도 zibServer와 연결한적 없는 경우)
             zibServer에서 수신한 key가 없는 경우.
             RTC에서 응답이 없거나 해당 key에 대한 address를 수신하지 못한 경우.
         1 : zibServer key로 RTC에서 zibServer address를 수신한 경우.
*/
    int connectToWan() { return 1;} // TODO

/**
@brief   connection이 여전히 유효한지 packet을 전송하여 응답여부를 확인한다.

@return  0 : 단말에 등록된 zibServer가 없는 경우.
             zibServer와의 연결이 끊어진 경우.
             zibServer에서 응답이 없는 경우.
         1 : zibServer에서 정상적인 응답이 온 경우.
*/
/*
    int sendPing()
    {
        if (_net._users.N1() < 1 || _net.GetIds().N() < 1) return 0;

        _net.Notify(0, zbNoti::none);

        return 1;
    }
*/

/**
@brief  1. 파일 전송, File ID를 입력받아 파일 목록에서 파일 경로를 찾아 서버에 업로드
        2. 업로드 결과를 받아 파일 목록의 백업 상태를 업데이트.

@param  fid[in] : file.list에 저장된 file id

@return 1 : 성공
*/
    int uploadFile(int fid)
    {
        if (_net.GetIds().N() < 1) return 0;

        _net.SendFile(0, 0, fid, true);
        return 1;
    }

/**
@brief  1. 여러 파일 전송, File ID의 list를 입력받아 파일 목록에서 파일 경로를 찾아 서버에 업로드
        2. 업로드 결과를 받아 파일 목록의 백업 상태를 업데이트.

@param  fid[in] : file.list에 저장된 file id

@return 0 : file list size가 0
        1 : 성공
*/
    int uploadFiles(vector<int> vFid)
    {
        if (_net.GetIds().N() < 1) return 0;

        if (vFid.size() < 1) return 0;

        for (auto fid : vFid)
            _net.SendFile(0, 0, fid, true);

        return 1;
    }

/**
@brief   집서버에 storage List 를 요청하여 user 가 가진 storage 와 집서버가 가진 user 의 storage 가 일치하는지 확인

@return  ??
*/
    int validateFileList() {return 1;} // TODO

////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////
    void quickSort(int first, int last, vector<int64>& files, vector<int>& v)
    {
        int pivot;
        int i;
        int j;
        int temp;

        if (first < last)
        {
            pivot = first;
            i = first;
            j = last;

            while (i < j)
            {
                // Decen
                while (files[v[i]] > files[v[pivot]])
                {
                    i++;
                }
                while (files[v[j]] <= files[v[pivot]] && j > first)
                {
                    j--;
                }
                if (i < j)
                {
                    temp = v[i];
                    v[i] = v[j];
                    v[j] = temp;
                }
/*
                // Incre
                while (files[v[i]] <= files[v[pivot]] && i < last)
                {
                    i++;
                }
                while (files[v[j]] > files[v[pivot]])
                {
                    j--;
                }
                if (i < j)
                {
                    temp = v[i];
                    v[i] = v[j];
                    v[j] = temp;
                }
*/
            }

            temp = v[pivot];
            v[pivot] = v[j];
            v[j] = temp;

            quickSort(first, j - 1, files, v);
            quickSort(j + 1, last, files, v);
        }
    }
////////////////////////////////////////
/**
@brief   안드로이드 내부 모든 저장소의 파일 목록을 시간순서로 정렬하여 Iterator 형태로 반환

@return  sortedFileList
*/
    vector<int> getTotalFileListDateSorted()
    {
        vector<int> v;
        if (_net._users.N1() < 1) return v;

        zbFiles& files = _net._users(0).strgs(0).files;
        if (files.N1() < 1) return v;

        v.reserve(files.N1());

        vector<int64> vFile;
        vFile.reserve(files.N1());
        for (int i = 0; i < files.N1(); ++i)
        {
            vFile.push_back(files(i).date.GetInt());
            if (files(i).state == zbFileState::bkup || files(i).state == zbFileState::bkuponly)
                v.push_back(i);
        }

        if (v.size() > 0) quickSort(0, v.size()-1, vFile, v);
        return v;
    }

/**
@brief  fileID 를 입력받아 파일 목록에서 해당 파일의 정보를 반환

@param  fid[in] : file id
        infos[out] : fid, file type, name(path), backup state등의 file 정보

@return  0 : file info 획득 실패
         1 : file info 획득 성공
*/
    int getFileInfo(int fid, FileInfos& infos)
    {
        if (_net._users.N1() < 1) return 0;

        zbFiles& files = _net._users(0).strgs(0).files;
        if (fid < 0) return 0;
        if (fid >= files.N1()) return 0;

        zbFile& file = files(fid);

        infos.type = (int)file.type;
        infos.state = (int)file.state;
        infos.date = file.date.GetInt();

        char name[200] = {0,};
        wcstombs(name, file.name.P(), file.name.Byte());
        infos.name.assign(name);

        return 1;
    }

/**
@brief  file ID 를 입력받아 서버에  파일을 요청하여  응답 파일을 지정된 download path 에 저장

@param  fid[in] : file id

@return -2 (not connected), -1 (id is out of range), 0 (time out) 1 (ok)
*/
    int requestFile(int fid)
    {
        return _net.RequestFile(0, 0, fid, TIME_OUT_SEC);
    }

/**
@brief  file ID 를 입력받아 서버에 파일을 요청하여 응답 파일을 지정된 cache path에 저장
        이는 download와 다르게 app에서 zibServer의 file들을 보여주기 위한 용도이다.
        때문에 requestFile(download)와는 별도로 구분

@param fid[in] : file id

@return -2 (not connected), -1 (id is out of range), 0 (time out) 1 (ok)
*/
    int requestCacheFile(int fid)
    {
        return _net.RequestCache(0, 0, fid, TIME_OUT_SEC);
    }

/**
@brief   file ID 를 입력받아 서버에  thumbnail Media 를 요청하여  응답 파일을 지정된 path 에 저장

@param fid[in] : file id

@return  -3 (not image), -2 (not connected), -1 (id is out of range), 0 (time out) 1 (ok)
*/
    int requestThumbnail(int fid)
    {
        return _net.RequestThumb(0, 0, fid, TIME_OUT_SEC);
    }

/**
@brief   갤러리에 새로 추가된 파일이나 삭제된 파일을 검사하여  파일목록을 업데이트

@return  ??
*/
    int updateFileList()
    {
        if (_net._users.N1() < 1) return 0;

        _net.UpdateFile(false);
        return 1;
    }

/**
@brief   1. 파일리스트에 백업 상태가 backUpNo 인 파일을 전부 집서버에 전송
         2. 전송 확인 절차를 거쳐 전송이 확인되면 파일리스트의 백업 상태를 업데이트

@return  ??
*/
    int backUpAll()
    {
        if (_net.GetIds().N() < 1) return 0;

        _net.UpdateFileOfList(0, 0); // default true
        return 1;
    }

/**
@brief   backUp이 모두 완료되었는지 확인.
         file list는 이미 update 되었기 때문에 file list의 가장 마지막 fid의 state가 back인지 확인

@return  1 : 마지막 fid의 state가 bkup
         0 : 마지막 fid의 state가 bkupno
*/
    int checkBackUpComplete()
    {
        if (_net._users.N1() < 1) return 0;

        return _net.checkStateOfLastFile(0, 0);
    }

/**
@brief  1. fileID 를 입력받아 device 의 (client) 파일 삭제
        2. 삭제가 완료되면 자신의 파일 목록을 업데이트하고 서버에 업데이트 사실을 알림
        3. 서버의 파일 목록 업데이트 확인

@param  fid[in] : file id

@return 
*/
    int deleteDeviceFile(int fid)
    {
        if (_net._users.N1() < 1) return 0;

        _net.DeleteFileClt(0, 0, fid);
        return 1;
    }

/**
@brief  1. fileID 를 입력받아 서버에 해당 파일의 삭제를 요청
        2. 삭제가 완료되면 삭제 완료 응답을 받아 자신의 파일 목록을 업데이트

@param  fid[in] : file id

@return 
*/
    int deleteServerFile(int fid)
    {
        if (_net.GetIds().N() < 1) return 0;

        _net.BanBkup(0, 0, fid, true);
        return 1;
    }

/**
@brief  fileID 를 입력받아 클라이언트에서 파일 백업이 가능한 상태로 변경
@param  fid[in] : file id
@return
*/
    int allowFileBackUp(int fid)
    {
        if (_net._users.N1() < 1) return 0;

        _net.LiftBanBkup(0, 0, fid);
        return 1;
    }

/**
@brief  1. fileID 를 입력받아 서버에 해당 파일의 삭제를 요청
        2. 삭제가 완료되면 삭제 완료 응답을 받고 디바이스 내 파일도 삭제
        3. 파일리스트 업데이트

@param  fid[in] : file id

@return 
*/
    int deleteBothFile(int fid)
    {
        if (_net.GetIds().N() < 1) return 0;

        _net.DeleteFileBoth(0, 0, fid, true);
        return 1;
    }

/**
@brief  APP Action에 의해서 file의 state가 변경된 경우, file list에 저장
@param
@return 
*/
    int saveFileList()
    {
        if (_net._users.N1() < 1) return 0;

        _net.SaveFileList(0, 0);
        return 1;
    }

/**
@brief  Registered User Info를 획득한다.

@param

@return 모든 Registered User의 name, mac, ip 정보
*/
    string getUsersInfo()
    {
        if (_net._users.N1() < 1) return "";

        ostringstream oss;
        zbUsers& users = _net._users;
        for (int i = 0; i < _net._users.N1(); ++i)
        {
            oss<<" * user["<<i<<"]\n";
            oss<<users(i).PrintInfo()<<"\n";
        }

        return oss.str();
    }

/**
@brief  Connected Device Info를 획득한다.

@param

@return 모든 Connected Device의 name, mac, ip 정보
*/
    string getCurrConnInfo()
    {
        if (_net.GetIds().N() < 1) return "";

        ostringstream oss;
        kmNetIds& ids = _net.GetIds();
        for (int i = 0; i < _net.GetIds().N(); ++i)
        {
            oss<<" * Connected Device ["<<i<<"]\n";
            oss<<ids(i).GetStr()<<"\n";
        }

        return oss.str();
    }

////////////////////////////////////////
////////////////////////////////////////
    // set path
    void setRootPath(string& path)
    {
        wchar  path_name[512] = {0,};

        mbstowcs(path_name, path.c_str(), path.length());
        _net._path.SetStr(path_name);
    };

    void setSrcPath(string& path)
    {
        wchar  path_name[512] = {0,};

        mbstowcs(path_name, path.c_str(), path.length());
        _net._srcpath.SetStr(path_name);
    };

    void setDlPath(string& path)
    {
        wchar  path_name[512] = {0,};

        mbstowcs(path_name, path.c_str(), path.length());
        _net._dwnpath.SetStr(path_name);
    };

    int getFileNum()
    {
        if (_net._users.N1() < 1) return 0;
    
        zbFiles& files = _net._users(0).strgs(0).files;
        return files.N1();
    };

    ///////////////////////////////////
    // windows procedure functions
protected:
    // create child window
    virtual void CreateChild(const zbNet::PathSet& pathSet, const string& deviceID, const string& deviceName)
    {
        setlocale(LC_ALL, "");

        // init net
        _net.Init(this, cbRcvNetStt, pathSet, deviceID, deviceName);
    };

    // print pkeys only for nks svr
    void PrintPkeys()
    {
        print("******** pkeys\n");
        if(_net._mode == zbMode::nks) { _net._nks.Print(); _net.SaveNks(); }
        if(_net._mode == zbMode::svr) _net.GetPkey().Print();
        if(_net._mode == zbMode::clt)
        {
            for(int i = (int)_net._users.N1(); i--;) _net._users(i).key.Print();
        }
    };


    ////////////////////////////////////
    // callback functions for net
    static int cbRcvNetStt(void* lnx, uchar ptc_id, char cmd_id, void* arg)
    {
        return ((zibLinux*)lnx)->cbRcvNet(ptc_id, cmd_id, arg);
    };
    int cbRcvNet(uchar ptc_id, char cmd_id, void* arg)
    {
        switch(ptc_id)
        {
        //case 0: return cbRcvNetPtcBrdc(cmd_id, arg);
        case 1: return cbRcvNetPtcCnnt(cmd_id, arg);
        case 2: return cbRcvNetPtcData(cmd_id, arg); 
        //case 3: return cbRcvNetPtcLrgd(cmd_id, arg);
        case 4: return cbRcvNetPtcFile(cmd_id, arg);
        }
        return -1;
    };
    int cbRcvNetPtcCnnt(char cmd_id, void* arg)
    {    
        int id = _net.GetCnntingId(cmd_id);

        if(cmd_id == 0) print("* connection has been requested (id: %d)\n", id);
        else            print("* connection has been accepted  (id: %d)\n", id);

        _net.GetId(id).Print(); print("\n");

        return 1;
    };
    int cbRcvNetPtcData(char cmd_id, void* arg)
    {
        uchar data_id = _net.GetDataId();

        switch(data_id)
        {
        case 0: cbRcvInfo(); break; // receive info
        //case 1: UpdateTbl(); break; // receive reqregist
        //case 2: UpdateTbl(); break; // receive acceptregist
        default:break;
        }
        return 1;
    };
    int cbRcvNetPtcFile(char cmd_id, void* arg)
    {
        return 1;
    };
    // * Note that when you receive rcvinfo, 
    // * you have to decide wheter to request register or not.
    void cbRcvInfo()
    {
        zbNetInfo& info = _net.GetLastRcvInfo(); // opposite's info

        // reqeust registration
        if(_net.FindUser(info.mac) < 0) // not registered
        if(_net._mode == zbMode::clt && info.mode == zbMode::svr)
        {
            _net.RequestRegist(info.src_id);
        }
    };

    ///////////////////////////////////////////////////
    // network functions

    // connect every network using broadcast
    int ConnectBroadcast()
    {
        //static kmThread thrd; thrd.Begin([](zibLinux* lnx)
        {
            //int n = lnx->_net.ConnectNew();
            //int n = _net.ConnectNew();

            //print("* number of devices connected : %d\n", n);

        }//,this);

        return _net.ConnectNew();
    };

    // connect with ip addr
    int Connect(kmAddr4 addr) { return _net.Connect(addr, 500.f); };

    // connect with input
    void ConnectIn()
    {
        char bufin[128];

        cout << "> input ip (ex : 10.114.75.49) : ";
        cin  >> bufin;

        kmAddr4 addr(bufin, _net._port);

        cout << "* connect to " << addr.GetStr().P() << endl;

        Connect(addr);
    }
};

/////////////////////////////////////////////////////////////////
void mainConvWC4to2(wchar* wc, ushort* c, const ushort& n) { for (ushort i = 0; i < n; ++i) { c[i] = (ushort)wc[i]; } };

int prot_id = 2;
void zibCli(zibLinux* linuxNet)
{
    sleep(2);
    while (1)
    {
        cout<<" =========================="<<endl;
        cout<<"Data(2), File(4) : ";

        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        cin>>prot_id;
        cout<<"Data(2), File(4) : ";
        cout<<prot_id<<endl;

        if (cin.fail())
        {
            cout << "ERROR -- You did not enter an integer"<<endl;

            // get rid of failure state
            cin.clear(); 

            // From Eric's answer (thanks Eric)
            // discard 'bad' character(s) 
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        sleep(5);
    }
}

/////////////////////////////////////////////////////////////////
// entry
int main() try
{
    zbNet::PathSet paths;
    paths.path = "/home/kktjin/backup/zibsvr";
    paths.srcpath = "/home/kktjin/backup/image";
    paths.dlpath = "/home/kktjin/backup/download";
    string deviceID = "AB12CD3412345678";
    string deviceName = "한글";

    zibLinux linuxNet;
    linuxNet.init(paths, deviceID, deviceName);

    linuxNet.connectToLan();
    linuxNet.connectWithZibSvr();

    // for debug
    //std::thread cli(zibCli, &linuxNet);

    sleep(1);
    linuxNet.updateFileList();
    cout<<linuxNet._net.PrintFileListTest(0,0);
    sleep(1);
    //linuxNet.uploadFile(0);
    linuxNet.backUpAll();
    sleep(10);
    cout<<linuxNet.getUsersInfo()<<endl;
    cout<<linuxNet.getCurrConnInfo()<<endl;
    //linuxNet.requestThumbnail(6);

/*
    zibLinux::FileInfos infos;
    linuxNet.getFileInfo(0, infos);
    cout<<infos.date<<endl;
    cout<<infos.type<<endl;
    cout<<infos.state<<endl;
    cout<<infos.name<<endl;

    sleep(1);
    cout<<linuxNet._net.PrintFileListTest(0,0);

    sleep(1);
    /////////////////
    cout<<"\n---- 시간측정 시작 ----"<<endl; //결과 출력

    time_t start, end;
    double result;
    int i, j;
    int sum = 0;

    start = time(NULL); // 시간 측정 시작

    vector<int> v = linuxNet.getTotalFileListDateSorted();

    end = time(NULL); // 시간 측정 끝

    result = (double)(end - start);
    cout<<"\n---- 시간측정 종료 ----"<<endl;
    cout<<"  걸린시간 : "<<result<<endl; //결과 출력
    /////////////////

    if (std::endian::native == std::endian::big)
        cout<<"빅엔디안"<<endl;
    else if (std::endian::native == std::endian::little)
        cout<<"리틀엔디안"<<endl;
*/

    while (1) {
        sleep(1);
        cout<<linuxNet._net.PrintFileListTest(0,0);
        sleep(10);
    }

    return 0;
}

catch(kmException e)
{
    print("* kmnet.cpp catched an exception\n");
    kmPrintException(e);
    system("pause");
    return 0;
}
