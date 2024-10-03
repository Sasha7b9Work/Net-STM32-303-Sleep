// 2024/01/11 11:02:46 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Modules/HI50/HI50.h"
#include "Hardware/HAL/HAL.h"
#include "Hardware/Timer.h"
#include "Hardware/HAL/HAL.h"
#include "Hardware/InterCom.h"
#include <cstdlib>


namespace HI50
{
    static const uint8 TURN_ON = 0x4f;
//    static const uint8 MEAS_AUTO = 0x44;
    static const uint8 MEAS_HI = 0x4D;

    struct State
    {
        enum E
        {
            IDLE,
            WAIT_TURN_ON,
            WAIT_MEASURE
        };
    };

    static State::E state = State::IDLE;

    static Measure distance;

    static bool is_exist = false;   // true, ���� ������ ��������� ��������� ���������

    static uint time_last_send = 0;

    static void SendRequestMeasure()
    {
        HAL_USART1::Send(MEAS_HI);

        time_last_send = TIME_MS;
    }
}


bool HI50::IsExist()
{
    return is_exist;
}


bool HI50::Init()
{
    HAL_USART1::SetModeHI50();

    state = State::WAIT_TURN_ON;

    HAL_USART1::Send(TURN_ON);

    TimeMeterMS meter;

    while (meter.ElapsedTime() < 500)
    {
        HAL_USART1::Update();

        if (state == State::WAIT_MEASURE)
        {
            is_exist = true;

            break;
        }
    }

    return is_exist;
}


void HI50::Update()
{
    if (!is_exist)
    {
        return;
    }

    switch (state)
    {
    case State::IDLE:
        HAL_USART1::Send(TURN_ON);
        state = State::WAIT_TURN_ON;
        break;

    case State::WAIT_TURN_ON:
        break;

    case State::WAIT_MEASURE:
        if (TIME_MS - time_last_send > 5000)
        {
            SendRequestMeasure();
        }
        break;
    }
}


void HI50::CallbackOnReceive(pchar message)
{
    switch (state)
    {
    case State::IDLE:
        break;

    case State::WAIT_TURN_ON:
        HAL_USART1::Send(MEAS_HI);
        state = State::WAIT_MEASURE;
        break;

    case State::WAIT_MEASURE:

        // ���� �������� ������ ��������� �� ����������

        static const int SIZE_BUFER = 128;

        char buffer_digits[SIZE_BUFER] = { '\0' };

        for (int i = 0; i < SIZE_BUFER; i++)
        {
            char buf[2] = { message[i], '\0' };
            std::strcat(buffer_digits, buf);

            if (buf[0] == 0)
            {
//                Display::SetString(buffer_digits);

                if (buffer_digits[0] == 'M')
                {
                    for (int index = 3; index < SIZE_BUFER; index++)
                    {
                        if (buffer_digits[index] == 'm')
                        {
                            buffer_digits[index] = '\0';

                            std::memmove(buffer_digits, buffer_digits + 3, (uint)(index - 3));

                            float value = (float)std::atof(buffer_digits);

                            distance.Set(Measure::Distance, value);

                            Measure measure;
                            measure.Set(Measure::Distance, value);

                            if (value != 0.0f)
                            {
                                HAL_USART1::SetModeHC12();

                                for (int counter = 0; counter < 5; counter++)
                                {
                                    InterCom::Send(measure, TIME_MS);
                                }

                                HAL_USART1::SetModeHI50();
                            }

                            SendRequestMeasure();

                            return;
                        }
//                        else if (buffer_digits[index] == '!')
//                        {
//                            Measure measure;
//                            measure.Set(Measure::Distance, -1.0f);
//
//                            HAL_USART1::SetModeHC12();
//
//                            InterCom::Send(measure, TIME_MS);
//
//                            HAL_USART1::SetModeHI50();
//
//                            SendRequestMeasure();
//                        }
                    }
                }
            }
        }

        break;
    }
}


bool HI50::GetMeasure(Measure *measure)
{
    if (distance.correct)
    {
        *measure = distance;

        return true;
    }

    return false;
}
