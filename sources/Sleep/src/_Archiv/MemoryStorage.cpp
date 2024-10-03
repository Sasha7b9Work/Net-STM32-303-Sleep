// 2023/09/08 22:14:01 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Storage/MemoryStorage.h"
#include "Hardware/Timer.h"
#include "Modules/W25Q80DV/W25Q80DV.h"


namespace MemoryStorage
{
    struct Page
    {
        static const int SIZE = W25Q80DV::SIZE_PAGE;

        Page(uint) { }

    private:

//        uint address;
    };

    // ���� ��������� ����� ������, ����� ����� �������� � ���
    namespace Cache
    {
        static const int SIZE = 512;        // ������ ��� ������ ������ (��� 512 ������ �������� �� 3256 ��, ��� 8192 - �� 3149 ��)
        static uint8 buffer[SIZE] __attribute__((section("CCM_DATA")));

        static uint begin = (uint)-1;               // ������ ������ � ����

        static bool IsDirty()
        {
            return begin >= W25Q80DV::SIZE;
        }

        static uint End()
        {
            uint end = begin + SIZE;

            if (end > W25Q80DV::SIZE)
            {
                end = W25Q80DV::SIZE;
            }

            return end;
        }

        static void MarkAsDirty()
        {
            begin = (uint)-1;
        }

        // ���������� true, ���� �������� ������ �� ������ address, �������� size
        static bool Consist(uint address, int size)
        {
            if (IsDirty() || begin > address)
            {
                return false;
            }

            return address + (uint)size < End();
        }

        // ��������� ���������� ������, ������� � ������ address
        static void FillFromMemory(uint address)
        {
            begin = address;

            uint size = W25Q80DV::SIZE - begin;

            if (size > SIZE)
            {
                size = SIZE;
            }

            static int counter = 0;
            counter++;

            MemBuffer<SIZE> mem_buf;

            uint8 *src = mem_buf.Read(begin);

            std::memcpy(buffer, src, SIZE);
        }

        static void Read(uint address, void *out, int size)
        {
            if (!Consist(address, size))
            {
                FillFromMemory(address);
            }

            uint shift = address - begin;

            std::memcpy(out, &buffer[shift], (uint)size);
        }

        static void Write(uint address, const Measurements &meas)
        {
            W25Q80DV::WriteBuffer<sizeof(meas)>(address, &meas);

            if (Consist(address, sizeof(meas)))
            {
                MarkAsDirty();
            }
        }
    }

    // ������������ ����� � ������ ��� ������ ����� ������
    static void PrepareForRecord(const Record &);

    static Record first_record((uint)-1);
    static Record last_record((uint)-1);
}


int Record::GetNextNumber()
{
    if (!MemoryStorage::last_record.IsExist())
    {
        return 1;
    }

    return MemoryStorage::last_record.GetMeasurements().number + 1;
}


void MemoryStorage::Init()
{
    for (uint address = 0; address < W25Q80DV::SIZE; address += Record::SIZE)
    {
        Record record(address);

        if (record.IsExist() &&
            !record.IsEmpty() &&                // ���� ���� ��� ���-�� ���� ��������
            !record.IsErased())                 // � �� �����
        {
            if (record.IsCorrect())
            {                                   // ������������ ������ � ��������� ������
                if (!first_record.IsExist())
                {
                    first_record = record;
                }

                if (!last_record.IsExist())
                {
                    last_record = record;
                }

                if (record.GetNumber() > last_record.GetNumber())
                {
                    last_record = record;
                }

                if (record.GetNumber() < first_record.GetNumber())
                {
                    first_record = record;
                }
            }
            else
            {
                record.Erase();         // ������� ������������ ������
            }
        }
    }
}


void MemoryStorage::Append(Measurements &meas)
{
    /*
    *       ������� ��������.
    *   1. ������ ������ ���������� ���������� �������, ������� ������������� ����� �������:
    *       - ���� ��������� ������, �� ���� ����� 1.
    *       - ���� � ��������� ��� ���� ������, �� ���� ����� ����� "����� ��������� ������ + 1"
    *   2. ���� ��������� ����� � ������:
    *       - ���� ������� ���, �� ���� ����� ����� 0
    *       - ���� ����� ��������� ������ ������ ������ ������ ������, �� ������ ����� ���� �� ��������� ������� � ������ ��������� �������
    */

    int number = Record::GetNextNumber();

    Record record(0);

    if (last_record.IsExist())
    {
        if (last_record.GetAddress() > first_record.GetAddress())   // ���� ������ ���� � ������ ������� - ��������� ��� ����� ������
        {
            record = last_record.GetNextRecord();

            if (record.GetAddress() < last_record.GetAddress())     // ���� ��������� ������ ��������� ����� ��������� �������
            {
                MemoryStorage::PrepareForRecord(record);
            }
        }
        else
        {

        }
    }

    record.Write(meas, number);
}


bool Record::IsEmpty()
{
    static bool first = true;
    static Measurements empty_meas;

    if (first)
    {
        first = false;

        std::memset(&empty_meas, 0xFF, sizeof(empty_meas)); //-V780
    }

    const Measurements &meas = GetMeasurements();

    return std::memcmp((void *)&meas, &empty_meas, sizeof(meas)) == 0; //-V1014
}


bool Record::IsErased()
{
    return GetMeasurements().number == 0;
}


bool Record::IsCorrect()
{
    const Measurements &meas = GetMeasurements();

    return (meas.control_field == 0) && (meas.CalculateCRC() == meas.crc);
}


void Record::Erase()
{
    W25Q80DV::WriteUInt(address, 0);

    if (MemoryStorage::Cache::Consist(address, sizeof(uint)))
    {
        MemoryStorage::Cache::MarkAsDirty();
    }
}


const Measurements &Record::GetMeasurements() const
{
    return value_meas.GetMeasurements(address);
}


const Measurements &Record::ValueMeasurements::GetMeasurements(uint addr) const
{
    Record::ValueMeasurements *pointer = (Record::ValueMeasurements *)this;

    if (!is_valid)
    {
        pointer->is_valid = true;

        MemoryStorage::Cache::Read(addr, &pointer->measurements, sizeof(measurements));
    }

    return measurements;
}


void Record::Write(Measurements &meas, int number)
{
    meas.number = number;
    meas.crc = meas.CalculateCRC();

    MemoryStorage::Cache::Write(address, meas);

    MemoryStorage::last_record = *this;
}


Record Record::GetNextRecord() const
{
    uint addr_next = address + SIZE;

    uint addr_end = addr_next + SIZE;

    if (addr_end >= W25Q80DV::SIZE)
    {
        addr_next = 0;
    }

    return Record(addr_next);
}


void MemoryStorage::PrepareForRecord(const Record &record)
{
    // ���� ������� ���� record �������� �� ������� ���� ����� ��������, �� ����� ������� ��� ��������

    int num_page = (int)(record.GetAddress() / W25Q80DV::SIZE_PAGE);
    int next_page = (int)((record.GetAddress() + Record::SIZE) / W25Q80DV::SIZE_PAGE);

    if (record.GetAddress() % W25Q80DV::SIZE_PAGE)
    {
        W25Q80DV::ErasePage(num_page);

        Cache::MarkAsDirty();
    }

    // ���� ������� ���� record ������� �� ���� ��������, � ��������� - �� ���������, �� ����� ������� ��������� ��������
    if (num_page != next_page)
    {
        W25Q80DV::ErasePage(next_page);

        Cache::MarkAsDirty();
    }
}
