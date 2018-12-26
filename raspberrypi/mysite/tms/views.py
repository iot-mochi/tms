import serial
import time
import functools
from django.http.response import HttpResponse
from django.shortcuts import render
from . import forms
from . import voice


def get_req(request):
    d = {
        'command': request.GET.get('command')
    }
    return render(request, 'get.html', d)

def manage(request):
    form = forms.ManageForm(request.GET or None)
    sbuf = ''
    result = ''
    if form.is_valid():
        cmd_str = request.GET.get('command')
        cmd_list = cmd_str.strip().split(" ")
        print(cmd_list)
        print(cmd_list[0])
        if cmd_list[0] == 'check' or cmd_list[0] == 'c':
            con = get_con()
            con.write(bytes('read', 'utf-8'))
            sbuf = con.readline().decode().strip();
            print(sbuf)
            result = check_state(sbuf)

        elif cmd_list[0] == 'knock' or cmd_list[0] == 'k':
            con = get_con()
            con.write(bytes('knock', 'utf-8'))
            sbuf = con.readline().decode().strip();
            print(sbuf)
            if check_knock(sbuf):
                result = "ノック応答あり"
            else:
                result = "ノック応答なし"

        elif cmd_list[0] == 'provoke' or cmd_list[0] == 'p':
            con = get_con()
            con.write(bytes('provoke', 'utf-8'))
            print("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa---------------")
            voice.play(cmd_list[1])

        message = cmd_str + " を実行しました"

    else:
        message = "エラーかも"

    d = {
        'form': form,
        'message': message,
        'result1': result,
        'result2': sbuf,
        }
    
    return render(request, 'index.html', d)


def check_state(censer_data):
    
    data_buf = censer_data.split(';')
    dic = {}
    for temp in data_buf:
        temp = temp.split(',')
        dic.update([(temp[0], temp[1])])

    using_st = ""
    current_st = ""
    
    if check_open(dic['closing']) != 1:
        if check_human(dic['human']) == 1:
            if check_wave(dic['wave']) == 1:
                using_st = "使用中"
            else:
                if check_bright(['brightness']) >= 1:
                    using_st = "使用中"
                else:
                    if check_temp(dic['temperature']) == 1:
                        using_st = "使用中"
                    else:
                        if check_humi(dic['humidity']) == 1:
                            using_st = "使用中"
                        else:
                            using_st = "空き"
        else:
            using_st = "密室殺人？"
    else:
        using_st = "空き"

    if check_open(dic['closing']) == 1:
        if check_press(dic['press']) != 1:
            if check_sound(dic['sound']) != 1:
                if check_odor(dic['odor']) != 1:
                    current_st = "安全"
                else:
                    current_st = "香り有益"
            else:
                current_st = "退出作業中"
        else:
            if check_sound(dic['sound']) == 1:
                if check_odor(dic['odor']) != 1:
                    current_st = "かなりやばい状況"
                else:
                    current_st = "パンツを履いていると祈りたい状況"
            else:
                current_st = "パンツを履いていると祈りたい状況"
    else:
        if check_press(dic['press']) != 1:
            if check_sound(dic['sound']) != 1:
                if check_odor(dic['odor']) != 1:
                    current_st = "放出中"
                else:
                    current_st = "放出準備"
            else:
                if check_odor(dic['odor']) == 1:
                    current_st = "放出完了"
                else:
                    current_st = "放出準備完了"
        else:
            if check_sound(dic['sound']) != 1:
                if check_odor(dic['odor']) == 1:
                    current_st = "パンツ脱ぎ中"
                else:
                    current_st = "パンツ履き中"
            else:
                if check_odor(dic['odor']) == 1:
                    current_st = "パンツ履いた直後"
                else:
                    current_st = "パンツ脱ぐ直前"

    return using_st, current_st

def check_knock(buf):
    buf = buf.split(',')
    if buf[1] == '1':
        return True
    return False

def check_open(state):
    if state == '1':
        return False
    return True

def check_human(state):
    if state == '1':
        return True
    return False

def check_press(state):
    if float(state) > 0.5:
        return True
    return False

def check_vibe(state):
    if state == '1':
        return True
    return False

def check_sound(state):
    if state == '1':
        return True
    return False

def check_wave(state):
    if float(state) < 40.0:
        return True
    return False

def check_odor(state):
    if int(state) > 300:
        return True
    return False

def check_temp(state):
    return True

def check_humi(state):
    return True

def check_bright(state):
    return True




@functools.lru_cache()
def get_con():
    con = serial.Serial('/dev/ttyACM0',9600)
    time.sleep(5)
    return con



