from flask import Flask, jsonify, abort, render_template, request
import json
from datetime import datetime
import random
import serial
app = Flask(__name__)
ser=serial.Serial("/dev/ttyAMA0",115200,timeout=1)
T_sensor='GET_T'
P_sensor='GET_P'
K_sensor='GET_K'
A_sensor='GET_A'
scale=[]


@app.route('/')
def welcome_page():
        return 'Welcom to BMP280 API !\n'

temperature=[]
pressure=[]

@app.route('/temp/', methods=['GET', 'POST'])
def api_temp():
        global temperature        #global variable
        ser.write(T_sensor.encode()) #send command to serial port
        T_value = (ser.readline().strip()).decode('ascii')
        now=datetime.now()
        T_value = float(T_value[2:-2])
        T_json={"Timestamp":now,"Unit":"C","Value":T_value}
        temperature.append(T_json)
        if request.method=='POST':
                return "", 202
        return jsonify({"Temperature values": temperature})

@app.route('/pres/', methods=['GET', 'POST'])
def api_pres():
        global pressure
        ser.write(P_sensor.encode())
        P_value = (ser.readline().strip()).decode('ascii')
        now=datetime.now()
        P_value=float(P_value[2:-2])
        P_json={"Timestamp":now,"Unit":"Pa","Value":P_value}
        pressure.append(P_json)
        if request.method=='POST':
                return "", 202
        return jsonify({"Pressure values": pressure})
@app.route('/scale/', methods=['GET'])
def api_scale():
        global scale
        ser.write(K_sensor.encode())
        scale = (ser.readline().strip()).decode('ascii')
        scale=float(scale[2:])
        return jsonify({"scale value": scale})
@app.route('/scale/<int:K>', methods=['POST'])
def api_scale_index(K=None):
        global scale
        if (K==None)|((K>9999)|(K<1000)):
                abort(404)
        ser.write(("SET_K="+str(K)).encode())
        return "", 202


@app.route('/temp/<int:index>', methods=['GET','DELETE'])
def api_temp_index(index):
        global temperature
        if index<len(temperature):
                if request.method == 'GET':
                        return jsonify({"index": index, "Temperature": temperature[index]})
                if request.method == 'DELETE':
                        del temperature[index]
                        return "", 205
        else:
                abort(404)

@app.route('/pres/<int:index>', methods=['GET','DELETE'])
def api_pres_index(index):
        global pressure
        if index<len(pressure):
                if request.method == 'GET':
                        return jsonify({"index": index, "Pressure": pressure[index]})
                if request.method == 'DELETE':
                        del pressure[index]
                        return "", 205
        else:
                abort(404)


@app.errorhandler(404)
def page_not_found(error):
        return render_template('page_not_found.html'), 404