// classification_cpu.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "openvino/cnn.h"
#include <Windows.h>
#include <stdio.h>
#include <chrono>
#include <omp.h>

bool grun = true;
typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::duration<double, std::ratio<1, 1000>> ms;
typedef std::chrono::duration<float> fsec;

DWORD WINAPI ThreadMain(LPVOID lp)
{
	double totalms = 0.0;
	int totalcount = 0;
	CNNClass c;
	//omp_set_dynamic(0);
	//omp_set_num_threads(1);
	c.Load("openvino/model/squeezenet1.1.xml", "openvino/model/squeezenet1.1.bin", "MainWork");
	if (c.err_msg != "") {
		printf("%s", c.err_msg);
		getchar();
		return -1;
	}

	auto t0 = Time::now();
	while (grun) {
		c.InsertImage();
		c.InsertImage();
		c.InsertImage();
		auto t1 = Time::now();
		fsec fs = t1 - t0;
		ms d = std::chrono::duration_cast<ms>(fs);
		totalms += d.count();
		t0 = Time::now();
		if (++totalcount == 100)
		{
			printf("ThreadMain fps=%.2lf\n", 1000*100/totalms);
			totalms = 0;
			totalcount = 0;
		}
		Sleep(1);//test
	}
	return 0;
}

DWORD WINAPI ThreadIdle(LPVOID lp)
{
	double totalms = 0.0;
	int totalcount = 0;
	CNNClass c;
	omp_set_dynamic(0);
	omp_set_num_threads(1);
	c.Load("openvino/model/squeezenet1.1.xml", "openvino/model/squeezenet1.1.bin", "IdlenWork");
	if (c.err_msg != "") {
		printf("%s", c.err_msg);
		getchar();
		return -1;
	}
	auto t0 = Time::now();
	while (grun) {
		c.InsertImage();
		c.InsertImage();
		c.InsertImage();
		auto t1 = Time::now();
		fsec fs = t1 - t0;
		ms d = std::chrono::duration_cast<ms>(fs);
		totalms += d.count();
		t0 = Time::now();
		if (++totalcount == 100)
		{
			printf("*****ThreadIdle fps=%.2lf\n", 1000 * 100 / totalms);
			totalms = 0;
			totalcount = 0;
		}
		//Sleep(1);//test
	}
	return 0;
}


int main()
{
	DWORD dwTrheadID1, dwTrheadID2;
	HANDLE h[2];
	h[0] = ::CreateThread(
		NULL,
		0,
		ThreadIdle,
		NULL,
		CREATE_SUSPENDED,
		&dwTrheadID1
	);
	::SetThreadPriority(h[0], THREAD_PRIORITY_LOWEST);

	h[1] = ::CreateThread(
		NULL,
		0,
		ThreadMain,
		NULL,
		CREATE_SUSPENDED,
		&dwTrheadID2
	);
	::SetThreadPriority(h[1], THREAD_PRIORITY_NORMAL);

	::ResumeThread(h[0]);
	::ResumeThread(h[1]);


	while (true) {
		char c = getchar();
		if (c == 'x' || c == 'X') {
			grun = false;
			break;
		}
	}

	::WaitForMultipleObjects(
		2,
		h,
		TRUE,
		INFINITE
	);

	::CloseHandle(h[0]);
	::CloseHandle(h[1]);

	printf("ok");
	getchar();
    return 0;
}

