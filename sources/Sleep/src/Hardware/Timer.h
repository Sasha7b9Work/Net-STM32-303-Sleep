#pragma once
#include "defines.h"


#define TIME_MS Timer::CurrentTime()


namespace Timer
{
    uint CurrentTime();

    void Delay(uint delayMS);
}



// ��������� ��� ������� �������
struct TimeMeterMS
{
    TimeMeterMS();

    // ���������� ������ �������
    void Reset();

    // ������� ����������� ������ � ������� ������ Reset()
    uint ElapsedTime();

    void PauseOnMS(uint);

    // ����� ms ����������� IsFinished() ���������� ������ true
    void FinishAfter(uint ms);

    bool IsFinished() const;

private:

    uint time_reset;        // �� ����� ������� ������������� ElapsedTime()
    uint time_finished;     // � ��� ����� IsFinished() ���������� ������ true
};
