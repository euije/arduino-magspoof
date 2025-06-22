#include <Arduino.h>
#include "MagSpoof.h"

MagSpoof::MagSpoof(uint8_t pinA, uint8_t pinB, uint8_t pinEnable, uint8_t clock, BPC bpc, Parity parity)
    : m_pinA(pinA)
    , m_pinB(pinB)
    , m_pinEnable(pinEnable)
    , m_clock(clock)
    , m_bpc(bpc)
    , m_parity(parity)
{
}

void MagSpoof::setup()
{
    pinMode(m_pinA, OUTPUT);
    pinMode(m_pinB, OUTPUT);
    pinMode(m_pinEnable, OUTPUT);
}

char MagSpoof::convertChar(char c)
{
    switch (m_bpc)
    {
        case BPC4:
        case BPC5:
            return c - '0';
        case BPC6:
        case BPC7:
            return c - ' ';
        default:
            return c;
    }
}

void MagSpoof::playBit(int sendBit, uint8_t& dir)
{
    dir ^= 1;
    digitalWrite(m_pinA, dir);
    digitalWrite(m_pinB, !dir);
    delayMicroseconds(m_clock);

    if (sendBit)
    {
        dir ^= 1;
        digitalWrite(m_pinA, dir);
        digitalWrite(m_pinB, !dir);
    }
    delayMicroseconds(m_clock);

}

void MagSpoof::playTrack(const char* track)
{
    int tmp, crc, lrc = 0;
    uint8_t dir = 0;

    int bl = m_bpc - (m_bpc % 2);

    // enable H-bridge
    digitalWrite(m_pinEnable, HIGH);

    // First put out a bunch of leading zeros.
    // TODO 25 sounds arbitrary
    for (int i = 0; i < 25; i++)
    {
        playBit(0, dir);
    }

    //
    for (int i = 0; track[i] != '\0'; i++)
    {
        crc = 1;
        tmp = convertChar(track[i]);

        for (int j = 0; j < bl; j++)
        {
            crc ^= tmp & 1;
            lrc ^= (tmp & 1) << j;
            playBit(tmp & 1, dir);
            tmp >>= 1;
        }
        if (m_bpc % 2)
        {
            playBit(m_parity == Odd ? crc : !crc, dir);
        }
    }

    // finish calculating and send last "byte" (LRC)
    tmp = lrc;
    crc = 1;
    for (int j = 0; j < bl; j++)
    {
        crc ^= tmp & 1;
        playBit(tmp & 1, dir);
        tmp >>= 1;
    }
    if (m_bpc % 2)
    {
        playBit(m_parity == Odd ? crc : !crc, dir);
    }

    // finish with 0's
    // TODO 5 * 5 (25) sounds arbitrary
    for (int i = 0; i < 5 * 5; i++)
    {
        playBit(0, dir);
    }

    digitalWrite(m_pinA, LOW);
    digitalWrite(m_pinB, LOW);
    digitalWrite(m_pinEnable, LOW);

}

void MagSpoof::playTrack(const char* track, float voltage)
{
    // 1) 전압 범위 클램핑
    if (voltage < 0.0f) voltage = 0.0f;
    if (voltage > 5.0f) voltage = 5.0f;
    // 2) 0~5V → 0~255 PWM 값으로 매핑
    uint8_t pwmValue = (uint8_t)round(voltage * 255.0f / 5.0f);

    int tmp, crc, lrc = 0;
    uint8_t dir = 0;
    int bl = m_bpc - (m_bpc % 2);

    // H-bridge Enable 핀에 PWM 출력 설정
    // (analogWrite는 내부적으로 Timer를 써서 PWM 생성)
    analogWrite(m_pinEnable, pwmValue);

    // 1) 선행 zero 비트
    for (int i = 0; i < 25; i++) {
        playBit(0, dir);
    }

    // 2) 데이터 전송 루프
    for (int i = 0; track[i] != '\0'; i++) {
        crc = 1;
        tmp = convertChar(track[i]);
        for (int j = 0; j < bl; j++) {
            crc ^= tmp & 1;
            lrc ^= (tmp & 1) << j;
            playBit(tmp & 1, dir);
            tmp >>= 1;
        }
        if (m_bpc % 2) {
            playBit(m_parity == Odd ? crc : !crc, dir);
        }
    }

    // 3) LRC 전송
    tmp = lrc;
    crc = 1;
    for (int j = 0; j < bl; j++) {
        crc ^= tmp & 1;
        playBit(tmp & 1, dir);
        tmp >>= 1;
    }
    if (m_bpc % 2) {
        playBit(m_parity == Odd ? crc : !crc, dir);
    }

    // 4) 마무리 zero 비트
    for (int i = 0; i < 25; i++) {
        playBit(0, dir);
    }

    // PWM 출력 종료
    analogWrite(m_pinEnable, 0);
    digitalWrite(m_pinA, LOW);
    digitalWrite(m_pinB, LOW);
}
