#include <opencv2/opencv.hpp>
#include <thread>
#include <string>
#include <chrono>
#include <mutex>
#include <iomanip>
#include <sstream>
#include "cvlibs.h"

using namespace cv;
using namespace std;
using namespace std::chrono;

//バッファ関係
const int n_buffer = 16;	//バッファサイズ
const int w = 480;
const int h = 360;
std::vector<Mat> buf(n_buffer, Mat(w, h, CV_8UC3));

//スレッド状態
bool is_alive = true;
bool is_ready = false;
mutex mtx;

//連番関係
const int n_frames = 32;
int i_current = -1;
int i_last = -2;
double fps = 30;

void load(void);

int main(void)
{
	//load関数をスレッドとして起動
	thread th = thread(load);

	//先にウィンドウを出しておくと初期フレームの描画遅延が小さくなる
	imshow("window", Mat(h, w, CV_8UC3, Vec3b(127, 127, 127)));
	while (!is_ready) { 
		this_thread::sleep_for(milliseconds(1)); 
	}

	//バッファが満タンになったらキー入力待機
	waitKey(0);

	//再生開始
	auto start = system_clock::now();
	while(waitKey(1) != 27)
	{
		if (!is_ready) continue;

		auto diff = system_clock::now() - start;
		long long ms = duration_cast<milliseconds>(diff).count();
		i_current = int(ms / (1000.0 / fps));

		if (n_frames <= i_current) break;
		
		mtx.lock();
		cout << this_thread::get_id() << "; i_current = " << i_current << endl;
		mtx.unlock();

		imshow("window", buf[i_current % n_buffer]);
	}

	//再生が終わったらキー入力待機
	waitKey(0);

	//スレッド終了を待機
	is_alive = false;
	if (th.joinable()) th.join();

	return 0;
}

void load(void)
{
	while (is_alive)
	{
		//最後にロードしたもの(i_last)が
		//再生中のフレーム(i_current)からバッファサイズ(n_bufffer)分先にあれば
		//これ以上バッファが足らないのでロードを待機する
		if (i_current + (n_buffer - 1) <= i_last)
		{
			is_ready = true;
		}
		else {

			//ロード位置が再生位置に抜かれた場合
			if (i_last < i_current) {
				i_last = i_current;
				is_ready = false;
			}

			int i_last_pp = ++i_last;

			//"jpg/img%03d.jpg"
			std::ostringstream oss;
			oss << "jpg/img" << setfill('0') << setw(3) << i_last_pp << ".jpg";
			string fname = oss.str();

			mtx.lock();
			cout << this_thread::get_id() << "; i_last_pp = " << i_last_pp << endl;
			mtx.unlock();

			buf[i_last_pp % n_buffer] = imread(fname);
		}
		this_thread::sleep_for(milliseconds(1));
	}
}