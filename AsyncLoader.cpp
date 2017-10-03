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

//�o�b�t�@�֌W
const int n_buffer = 16;	//�o�b�t�@�T�C�Y
const int w = 480;
const int h = 360;
std::vector<Mat> buf(n_buffer, Mat(w, h, CV_8UC3));

//�X���b�h���
bool is_alive = true;
bool is_ready = false;
mutex mtx;

//�A�Ԋ֌W
const int n_frames = 32;
int i_current = -1;
int i_last = -2;
double fps = 30;

void load(void);

int main(void)
{
	//load�֐����X���b�h�Ƃ��ċN��
	thread th = thread(load);

	//��ɃE�B���h�E���o���Ă����Ə����t���[���̕`��x�����������Ȃ�
	imshow("window", Mat(h, w, CV_8UC3, Vec3b(127, 127, 127)));
	while (!is_ready) { 
		this_thread::sleep_for(milliseconds(1)); 
	}

	//�o�b�t�@�����^���ɂȂ�����L�[���͑ҋ@
	waitKey(0);

	//�Đ��J�n
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

	//�Đ����I�������L�[���͑ҋ@
	waitKey(0);

	//�X���b�h�I����ҋ@
	is_alive = false;
	if (th.joinable()) th.join();

	return 0;
}

void load(void)
{
	while (is_alive)
	{
		//�Ō�Ƀ��[�h��������(i_last)��
		//�Đ����̃t���[��(i_current)����o�b�t�@�T�C�Y(n_bufffer)����ɂ����
		//����ȏ�o�b�t�@������Ȃ��̂Ń��[�h��ҋ@����
		if (i_current + (n_buffer - 1) <= i_last)
		{
			is_ready = true;
		}
		else {

			//���[�h�ʒu���Đ��ʒu�ɔ����ꂽ�ꍇ
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