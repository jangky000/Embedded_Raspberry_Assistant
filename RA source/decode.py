# -*- coding: utf-8 -*-

# 옵션 없으면 종료
import sys
from datetime import datetime, timedelta
import re

if len(sys.argv) == 1:
  print('error python arg')
  exit(1)

cmd = ' '.join(sys.argv[1:])

# print('raw:', cmd)

# 날짜 처리
# ====================================================================================================
r2=''

def Date_Process():
    global cmd
    time_now = datetime.now()

    p = re.compile(r'(?P<day>\d{1,3})일')
    isdate = p.search(cmd)

    Days = ['그저께', '그제', '어제', '오늘', '내일', '모레', '낼모레'] # 모레 == 낼모레 // 엊그제 != 그제
    if isdate:
        p2 = re.compile(r'(?P<month>\d{1,3})월 ?(?P<day>\d{1,2})일')
        month_day = p2.search(cmd)
        if month_day:
            # print('check',month_day.group())
            r1 = month_day.group()
            month = month_day.group('month')
            day = month_day.group('day')
            try:
                r2 = str(datetime(time_now.year, int(month), int(day)).date()).replace('-', '')
            except Exception as e:
                print('0 존재하지 않는 날짜, month day 확인')
                exit(2)
            # r2 = datetime(time_now.year, month, day).replace('-', '')
            cmd = cmd.replace(r1, r2)
        else:
            # print('check',isdate.group())
            r1 = isdate.group()
            day = isdate.group('day')
            # print(type(int(day)))
            # print(type(time_now.year))
            try:
                r2 = str(datetime(time_now.year, time_now.month, int(day)).date()).replace('-', '')
                # datetime(time_now.year, time_now.month, int(day))
            except Exception as e:
                print('0 존재하지 않는 날짜, day 확인')
                exit(2)
            cmd = cmd.replace(r1, r2)

    elif any(k in cmd for k in Days):
        # global time_now
        cmd = cmd.replace("그저께", str((time_now+timedelta(days=-2)).date()).replace('-', '') )
        cmd = cmd.replace("그제", str((time_now+timedelta(days=-2)).date()).replace('-', '') )
        cmd = cmd.replace("어제", str((time_now+timedelta(days=-1)).date()).replace('-', '') )
        cmd = cmd.replace("오늘", str((time_now+timedelta(days=0)).date()).replace('-', '') )
        cmd = cmd.replace("내일", str((time_now+timedelta(days=1)).date()).replace('-', '') )
        cmd = cmd.replace("모레", str((time_now+timedelta(days=2)).date()).replace('-', '') )
        cmd = cmd.replace("낼모레", str((time_now+timedelta(days=2)).date()).replace('-', '') )
    # else:
        # print('0 n월 n일, 혹은 n일의 형식이 아닙니다')
        # exit(2)
    # print(cmd)

# 일정 조회 / 추가 / 삭제
# ========================================================================================================
def Schedule():
    # print('schedule', cmd)
    INSERT_SCHEDULE = ['추가', '기록', '작성', '만들기']
    DELETE_SCHEDULE = ['삭제', '제거', '지우기']

    p = re.compile(r'(?P<date>\d{8})')
    date = p.search(cmd)

    if any(k in cmd for k in INSERT_SCHEDULE):
        if date:
            print('121 %s 일정 추가'%date.group('date'))
        else:
            print('120 일정 추가, 다음 명령으로 날짜를 입력하세요')
    elif any(k in cmd for k in DELETE_SCHEDULE):
        if date:
            print('131 %s 일정 삭제'%date.group('date'))
        else:
            print('130 일정 삭제, 다음 명령으로 날짜를 입력하세요')
    else:
        if date:
            print('111 %s 일정 조회'%date.group('date'))
        else:
            print('110 일정 조회, 다음 명령으로 날짜를 입력하세요')

# 내부 검색 외부 검색 판단
# =========================================================================================================
External_Search = ['유튜브', '구글', '네이버', '날씨', '뉴스']
if any(k in cmd for k in External_Search):
    print('2 외부검색')
    exit(0)
elif '일정' in cmd:
    # print('내부 검색')
    # print(cmd)
    Date_Process()
    Schedule()
    # print('1', cmd)
    exit(0)
else:
    Date_Process()
    p = re.compile(r'(?P<date>\d{8})')
    date = p.search(cmd)
    if date:
        print('101 %s 일정 날짜'%date.group('date'))
    else:
        print('0 %s 처리되지 않은 명령어' %cmd)
    
    
    exit(0)

print('실행되지 않는 영역')
