# ==================================================================
# 원격
from selenium import webdriver as wd
import urllib
from bs4 import BeautifulSoup
from urllib.request import urlopen
from time import sleep

STAT_DRIVER = 0
driver = None

def driverStart():
    global driver
    global STAT_DRIVER
    driver = wd.Chrome('static/chromedriver.exe')
    
    driver.implicitly_wait(1)
    STAT_DRIVER = 1

def youtubeShow(search):
    global driver
    url = 'https://www.youtube.com/results?search_query='
    keyword = urllib.parse.quote('%s' %search)
    f_url = url+keyword
    driver.get( f_url )
    driver.implicitly_wait(2)

def googleShow(search):
    global driver
    f_url=''
    if('뉴스' in search):
        f_url = 'https://news.google.com/?hl=ko&gl=KR&ceid=KR%3Ako'
    else:
        url = 'https://www.google.com/search?&q='
        keyword = urllib.parse.quote('%s' %search)
        f_url = url+keyword
    driver.get( f_url )
    driver.implicitly_wait(2)

def naverShow(search):
    global driver
    f_url=''
    if('뉴스' in search):
        f_url = 'https://news.naver.com/'
    elif('날씨' in search):
        f_url = 'https://weather.naver.com/'
    else:
        url = 'https://search.naver.com/search.naver?query='
        keyword = urllib.parse.quote('%s' %search)
        f_url = url + keyword
    driver.get(f_url)
    driver.implicitly_wait(2)

def daumShow(search):
    global driver
    f_url=''
    if('뉴스' in search):
        f_url = 'https://media.daum.net/'
    else:
        url = 'https://search.daum.net/search?w=tot&q='
        keyword = urllib.parse.quote('%s' %search)
        f_url = url + keyword
    driver.get(f_url)
    driver.implicitly_wait(2)

def driverClose():
    global driver
    global STAT_DRIVER
    driver.close()
    STAT_DRIVER = 0
# ==================================================================
# 크롤링
def Weather_Table_HTML():
    url_base = 'https://weather.naver.com/main/wetrMain.nhn' 
    page = urlopen( url_base ) # url 요청, 결과는 html
    soup = BeautifulSoup(page, 'html.parser')
    weather = soup.select('#content > div.m_zone1 > table')

    cols = weather[0].select('col')
    for col in cols:
        col.extract()

    icons = weather[0].select('.ico')
    for ico in icons:
        ico.extract()

    tables = weather[0].select('.info')
    tables[0]['colspan']='2'

    weather = str(weather[0])
    weather= "<h3>날씨 테이블 from Naver</h3>" + weather.replace('border="0"', 'border="1"', 1)

    # print(weather)
    return weather

def Weather_Report_HTML():
    url_base = 'https://weather.naver.com/news/wetrNewsList.nhn' 
    page = urlopen( url_base ) # url 요청, 결과는 html
    soup = BeautifulSoup(page, 'html.parser')
    report_url = soup.select('.tit > a')
    # report_url = 
    # print(report_url[0]['href'])
    url2 = report_url[0]['href']

    page2 = page = urlopen( url2 )
    soup2 = BeautifulSoup(page2, 'html.parser')
    reports = soup2.select('div#articleBodyContents')

    # imgs = reports[0].select('img')
    # for img in imgs:
    #     img.extract()
    reports = '<h3>날씨 리포트 from Naver</h3>' + str(reports[0])
    # print(reports)
    return reports

def Google_News_HTML():
    url_base = 'https://news.google.com/?hl=ko&gl=KR&ceid=KR:ko' 
    page = urlopen( url_base ) # url 요청, 결과는 html
    soup = BeautifulSoup(page, 'html.parser')
    news = soup.select('h3 > a.DY5T1d')
    news = [ new.string.strip() for new in news]

    html = '<h3>뉴스 헤드라인 from Google</h3><ul>'
    for new in news:
        html += '<li>%s</li>'%new
    html += '</ul>'

    # print(html)
    return html

# ==================================================================
# 명령 분석
def cmdDecode( cmd ):
    global STAT_DRIVER
    
    PCS = ['컴퓨터에서', '컴퓨터에', 'pc 에서', 'pc 에','pc에서', 'pc에', 'PC 에서', 'PC 에', 'PC에서', 'PC에']
    if any(k in cmd for k in PCS):
        # 드라이버 켜기
        if(STAT_DRIVER == 0):
            driverStart()

        for pc in PCS:
            if(pc in cmd):
                target = cmd.replace(pc, '', 1)
                break

        # PCS = ['컴퓨터에서', '컴퓨터에', 'PC에서', 'PC에']
        REMOTES = ['켜 줘', '켜줘', '띄워 줘', '띄워줘', '띄워', '검색해 줘', '검색해줘', '검색해', '검색']
        for remote in REMOTES:
            if(remote in cmd):
                target = target.replace(remote, '', 1)
                break

        try:
            if('유튜브' in target):
                YOUS = ['유튜브에서', '유튜브에', '유튜브']
                for you in YOUS:
                    if(you in cmd):
                        target = target.replace(you, '', 1)
                        break
                youtubeShow(target)
            elif('구글' in target):
                GOOS = ['구글에서', '구글에', '구글']
                for goo in GOOS:
                    if(goo in cmd):
                        target = target.replace(goo, '', 1)
                        break
                googleShow(target)
            elif('네이버' in target):
                NAVS = ['네이버에서', '네이버에', '네이버']
                for nav in NAVS:
                    if(nav in cmd):
                        target = target.replace(nav, '', 1)
                        break
                naverShow(target)
            elif('다음' in target):
                DAUS = ['다음에서', '다음에', '다음']
                for dau in DAUS:
                    if(dau in cmd):
                        target = target.replace(dau, '', 1)
                        break
                daumShow(target)
            else:
                # 디폴트는 네이버 검색
                naverShow(target)

        except Exception as e:
            STAT_DRIVER = 0
            cmdDecode(cmd)

        return 'remote'

    else:
        if('날씨' in cmd):
            return 'weather'
        elif('뉴스' in cmd):
            return 'news'
        return '정의되지 않은 명령'
        pass
        

# ======================================================================
#  서버 소켓
import socket

def serverSocket():
    Server_IP = '169.254.33.230'
    Server_PORT = 8578

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((Server_IP, Server_PORT))

    while True:
        server_socket.listen(0)
        print('클라이언트 대기중')
        client_socket, addr = server_socket.accept()
        print('접속 완료')
        while True:
            data = client_socket.recv(65535).decode('utf-8')
            if(len(data) == 0): continue
            print(type(data))
            print('[%s]'%data)
            if data == '연결 해제':
                break
            what = cmdDecode(data) # send remote, weather, ..
            client_socket.send(what.encode('utf-8'))
            # sleep(10)
            if(what == 'news'):
                client_socket.send(Google_News_HTML().encode('utf-8')) # 알아서 받을 수 있게 끊어서 보내준다!
            elif(what == 'weather'):
                client_socket.send(Weather_Table_HTML().encode('utf-8'))
                client_socket.send(Weather_Report_HTML().encode('utf-8'))
            # sleep(10)
            client_socket.send("done".encode('utf-8'))

        client_socket.close()
        print('클라이언트가 떠났습니다')
        # driverClose()

        # if input("재연결을 원하면 아무 문자, 재연결을 끝내려면 quit: ") == 'quit':
        #     break

    server_socket.close()
    print('서버 해제')
# ===================================================================

serverSocket()