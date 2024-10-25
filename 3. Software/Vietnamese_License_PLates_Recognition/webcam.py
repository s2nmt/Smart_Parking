import tkinter as tk
from PIL import Image, ImageTk
import cv2
import torch
import time
import function.helper as helper
import function.utils_rotate as utils_rotate
import pandas as pd
from paho.mqtt import client as mqtt_client
import threading
import requests
import re
class LicensePlateApp:
    def __init__(self, root):
        self.root = root
        self.broker = ''
        self.port = 
        self.username = ''
        self.password = ''

        self.topicPublish = "device/NFC"
        self.topicSubcribe = "server/NFC"

        self.mqtt_client = mqtt_client.Client()
        self.mqtt_client.username_pw_set(username=self.username, password=self.password)
        self.mqtt_client.on_connect = self.on_connect
        self.mqtt_client.on_message = self.on_message
        self.mqtt_client.connect(self.broker, self.port)

        # Start MQTT thread
        self.mqtt_thread = threading.Thread(target=self.mqtt_loop)
        self.mqtt_thread.daemon = True
        self.mqtt_thread.start()


        self.camera_on = True
        self.last_saved_time = None
        self.list_read_plates = set()
        self.last_recognized_plate = "unknown"
        self.last_plate = "unknown"
        self.code = None
        self.counterUnknow = 0
        self.flagSubcribe = 0
        self.publishclose = 0

        self.create_widgets()
        self.load_models()
        self.open_video_capture()
        self.update_labels()
        self.update_frame()

    def on_connect(self, client, userdata, flags, rc):
        print("Connected to MQTT broker with result code " + str(rc))
        self.mqtt_client.subscribe(self.topicSubcribe)

    def on_message(self, client, userdata, msg):
        message = msg.payload.decode()
        message = re.sub(r'^CODE: ', '', message)
        message = message.rstrip('\x00')
        print("Received message:", message)
        if self.camera_on:
            self.code=message
            self.flagSubcribe = 1
            self.last_plate = "unknown"
            self.info_text.delete('1.0', tk.END)
        else:
            self.info_text.delete('1.0', tk.END)
            self.info_text.insert(tk.END,"name:\nlicence_plate:\nCODE:"+message)

    def mqtt_loop(self):
        self.mqtt_client.loop_forever()

    def publish_message(self, message):
        if(message == "CLOSE"):
            if(self.publishclose == 1):
               return
            else:
                self.publishclose = 1
        else:
            self.publishclose = 0
        self.mqtt_client.publish(self.topicPublish, message)
        print(f"publish data: {message}")
       
    def create_widgets(self):
        self.root.title("License Plate Detection")

        screenWidth = self.root.winfo_screenwidth()
        screenHeight = self.root.winfo_screenheight()
        # setup  width and height screen
        width = 1000
        height = 600

        #calculate the x and y coordinates
        x_Coordinate = (screenWidth - width) // 2
        y_Coordinate = (screenHeight - height) // 2 - 20 # trừ 80n đề khung màn hình cao hơn

        root.geometry(f"{width}x{height}+{x_Coordinate}+{y_Coordinate}")
        root.resizable(width=False, height=False)

        self.configureRowsAndColumns(root, 12, 14, 10)

        titleMain = "QUẢN LÝ BÃI GIỮ XE"
        self.titleMainLabel = tk.Label(root, text=titleMain,fg='black', font=('cambria', 20, 'bold'), width=25)
        self.titleMainLabel.grid(row=1,column=0, columnspan=12,sticky='nsew',padx=0,pady=(0,0))
    
        self.titleMainLabel = tk.Label(root, text="Nguyễn Minh Tuấn, Phan Tấn Quốc",fg='black', font=('cambria', 10), width=25)
        self.titleMainLabel.grid(row=2,column=0, columnspan=12,sticky='nsew',padx=0,pady=(0,0))

        background_color = root.cget('bg')
        self.camera_frame = tk.Frame(self.root, bg=background_color, width=20, bd=1)
        self.camera_frame.grid(row=3, column=1, rowspan=6, columnspan=9, sticky='nsew', padx=10, pady=5)

        self.label = tk.Label(self.camera_frame)
        self.label.grid()
        self.toggle_button = tk.Button(self.root, text="Pause", command=self.toggle_camera,bd=0,width=1)
        self.toggle_button.grid(row=3,column=9,columnspan=2,sticky='nsew',padx=5,pady=5)

        # self.insert_button = tk.Button(self.root, text="Insert",bd=0,width=1,command=self.insert_license_plate)
        # self.insert_button.grid(row=3,column=9,sticky='nsew',padx=5,pady=5)
        # self.insert_button.config(state="disabled")

        self.plate_textbox = tk.Entry(self.root, font=("Helvetica", 16),width=15)
        self.plate_textbox.grid(row=4,column=9,columnspan=2,sticky='nsew',padx=0,pady=10)
        
        self.date_label = tk.Label(self.root, text="Date: ")
        self.date_label.grid(row=5,column=9,columnspan=2,sticky='nsew',padx=0,pady=0)

        self.info_label = tk.Label(self.root, text="Thông Tin Khách Hàng")
        self.info_label.grid(row=6,column=9,columnspan=2,sticky='nsew',padx=0,pady=0)

        self.info_text = tk.Text(self.root,width=10,height=5)
        self.info_text.grid(row=7, column=9,columnspan=2, sticky='nsew')
        
        self.row = tk.Button(self.root, text="",bd=0,width=5)
        self.row.grid(row=8,column=10,sticky='nsew',padx=0,pady=0)

        self.row1 = tk.Button(self.root, text="",bd=0,width=5)
        self.row1.grid(row=8,column=0,sticky='nsew',padx=0,pady=0)

        self.exit_button = tk.Button(self.root, text="Exit", command=self.exit_application,bd = 0)
        self.exit_button.grid(row=8,column=9,columnspan=2,sticky='nsew',padx=0,pady=0)

    def configureRowsAndColumns(self, root, num_rows, num_columns, weight_value):
        """
        Phân chia màn hình dưới dạng lưới với số hàng và cột được chỉ định,
        với trọng số được chỉ định để dễ dàng căn chỉnh
        """
        for i in range(num_rows):
            if(i == 0 or i == num_rows - 1):
                root.grid_rowconfigure(i, weight = weight_value//2)
            else:
                root.grid_rowconfigure(i, weight=weight_value)
        for j in range(num_columns):
            if(j == 0 or j == num_columns - 1):
                root.grid_columnconfigure(j, weight=weight_value//2)
            else:
                root.grid_columnconfigure(j, weight=weight_value)
    def load_models(self):
        self.yolo_LP_detect = torch.hub.load('yolov5', 'custom', path='model/LP_detector_nano_61.pt', force_reload=True, source='local')
        self.yolo_license_plate = torch.hub.load('yolov5', 'custom', path='model/LP_ocr_nano_62.pt', force_reload=True, source='local')
        self.yolo_license_plate.conf = 0.60
    
    def open_video_capture(self):
        self.vid = cv2.VideoCapture(0)
        
    def toggle_camera(self):
        self.camera_on = not self.camera_on
        if self.camera_on:
            self.toggle_button.config(text="Pause", command=self.toggle_camera)
        else:
            self.toggle_button.config(text="Start", command=self.toggle_camera)
        self.info_text.delete('1.0', tk.END)
    def update_labels(self):
        current_time = time.strftime("%Y-%m-%d %H:%M:%S")
        self.date_label.config(text="Date: " + current_time)
        self.root.after(1000, self.update_labels)
    def exit_application(self):
        self.vid.release()
        cv2.destroyAllWindows()
        self.root.destroy()
    def insert_license_plate(self):
        text_data = self.info_text.get("1.0", "end-1c")

        # Phân tích cú pháp dữ liệu và tạo đối tượng từ điển
        data = {}
        for line in text_data.split('\n'):
            if ':' in line:  # Kiểm tra xem dòng có chứa ít nhất một dấu hai chấm không
                key, value = line.split(":", 1)  # Tách key và value
                data[key] = value
            else:
                print("Invalid line:", line)
        self.license_data.insert_one(data)
        print(f"Data inserted:{data}")
        self.info_text.delete('1.0', tk.END)
        self.info_text.insert(tk.END,"Thêm Dữ Liệu Thành Công")

        
    def update_frame(self):
        if self.camera_on:
            ret, frame = self.vid.read()
            # self.insert_button.config(state="disabled")
        else:
            ret, frame = False, None
            # self.insert_button.config(state="normal")

        if ret:
            plates = self.yolo_LP_detect(frame, size=640)
            list_plates = plates.pandas().xyxy[0].values.tolist()
            lp = "unknown"
            flag = 0
            last_lp = self.last_recognized_plate
            for plate in list_plates:
                x = int(plate[0])
                y = int(plate[1])
                w = int(plate[2] - plate[0])
                h = int(plate[3] - plate[1])
                crop_img = frame[y:y + h, x:x + w]
                cv2.rectangle(frame, (int(plate[0]), int(plate[1])), (int(plate[2]), int(plate[3])), color=(0, 0, 225), thickness=2)
                cv2.imwrite("crop.jpg", crop_img)
                rc_image = cv2.imread("crop.jpg")
                for cc in range(0, 2):
                    for ct in range(0, 2):
                        lp_candidate = helper.read_plate(self.yolo_license_plate, utils_rotate.deskew(crop_img, cc, ct))
                        if lp_candidate.lower() != "unknown":
                            # self.list_read_plates.add(lp_candidate)
                            cv2.putText(frame, lp_candidate, (int(plate[0]), int(plate[1] - 10)), cv2.FONT_HERSHEY_SIMPLEX, 0.9, (36, 255, 12), 2)
                            flag = 1
                            lp = lp_candidate
                            self.counterUnknow = 0
                            break
                    if flag == 1:
                        break

            if flag == 1:
                if lp == last_lp:
                    self.counter += 1
                    if self.counter >= 5:  # License plate detected 10 times consistently
                        if (lp != self.last_plate):
                            if(self.flagSubcribe == 1):
                                flag_find = 0
                                try:
                                    response = requests.get('http://13.228.24.205:3000/datauser')
                                    if response.status_code == 200:
                                        # print(response.json())  # Convert response content to JSON format
                                        print(lp)
                                        print(type(self.code))
                                        print(type(lp))
                                        for item in response.json():
           
                                            if item.get('code') == self.code and  item.get('licence_plate') == lp:
                                                
                                                self.info_text.delete('1.0', tk.END)
                                                self.info_text.insert(tk.END,"Tên: " +  item.get('name') + "\nBiển số: "+item.get('licence_plate'))
                                                self.publish_message("OPEN")
                                                if(item.get('state') == "indoor"):
                                                    state = "outdoor"
                                                else:
                                                    state = "indoor"
                                                current_time = time.strftime("%Y-%m-%d %H:%M:%S")
                                                data_to_send = {
                                                    'users': item.get('users'),
                                                    'licence_plate': item.get('licence_plate'),
                                                    'time': current_time,
                                                    'state': state
                                                }
                                                data_to_update = {
                                                    'state': state
                                                }
                                                try:
                                                    url = 'http://13.228.24.205:3000/datauser/'  + item.get('_id')
                                                    response = requests.post('http://13.228.24.205:3000/stateuser', json=data_to_send)
                                                    response.raise_for_status()  # Raise an exception for HTTP errors
                                                    print('Success:', response.text.encode('utf-8'))  # Encode response content to UTF-8

                                                    response_update = requests.put(url, json=data_to_update)
                                                    if response_update.status_code == 200:
                                                        print("success")
                                                    else:
                                                        print("fail")

                                                    # Xử lý dữ liệu trả về nếu cần
                                                except requests.exceptions.RequestException as err:
                                                    print('Error:', err)
                                            # Xử lý lỗi nếu có
                                                flag_find = 1
                                                break
                                    else:
                                        print('Error:', response.status_code)
                                        # Handle other status codes if needed
                                except requests.exceptions.RequestException as err:
                                    print('Error:', err)
                                self.flagSubcribe = 0    
                                self.code=None
                                if(flag_find == 0):
                                    self.publish_message("CLOSE")
                                    self.flagSubcribe = 0
                                    self.last_plate = "unknown"
                                    # self.insert_to_mongodb(lp,"fail")
                                    self.code=None
                                    self.info_text.delete('1.0', tk.END)
                                    self.info_text.insert(tk.END,"Sai thông tin \nVui lòng thử lại")
                                # print(query)
                                # for document in cursor:
                                    
                                #     print(document['CODE'])
                                #     print(self.code)
                                #     if document['CODE'] == self.code and document['licence_plate'] == lp:
                                #         self.publish_message("OPEN")
                                #         # self.insert_to_mongodb(lp,"success")
                                #         self.info_text.delete('1.0', tk.END)
                                #         self.info_text.insert(tk.END,"Tên: " +  document['name'] + "\nBiển số: "+document['licence_plate'])
                                #         if(document['state'] == "indoor"):
                                #             self.license_data.update_one(
                                #             {"licence_plate": document['licence_plate']}, 
                                #             {"$set": {"state": "outdoor"}})
                                #         else :
                                #             self.license_data.update_one(
                                #             {"licence_plate": document['licence_plate']}, 
                                #             {"$set": {"state": "indoor"}})
                                #         self.flagSubcribe = 0
                                #         flag_find = 1
                                #         self.code=None

                                # if(flag_find == 0):
                                #     self.publish_message("CLOSE")
                                #     self.flagSubcribe = 0
                                #     self.last_plate = "unknown"
                                #     # self.insert_to_mongodb(lp,"fail")
                                #     self.code=None
                                #     self.info_text.delete('1.0', tk.END)
                                #     self.info_text.insert(tk.END,"Sai thông tin \nVui lòng thử lại")
                                    
                                self.last_plate = lp
                        self.counter = 0  # Reset counter

                else:
                    self.counter = 1  # Reset counter if the recognized plate changes
            else:
                self.counter = 0  # Reset counter if no plate detected
                if(lp == "unknown"):
                    self.counterUnknow += 1
                    # print(self.counterUnknow)
                    if self.counterUnknow == 5:
                        self.publish_message("CLOSE")
                        self.last_plate = "unknown"
                        self.counterUnknow = 0
                        self.info_text.delete('1.0', tk.END)
            self.last_recognized_plate = lp

            rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            img = ImageTk.PhotoImage(Image.fromarray(rgb_frame))
            self.label.img = img
            self.label.config(image=img)

            self.plate_textbox.delete(0, tk.END)
            self.plate_textbox.insert(0, lp)

            current_time = time.strftime("%Y-%m-%d %H:%M:%S")
            if self.last_saved_time is None or current_time != self.last_saved_time:
                self.last_saved_time = current_time

        self.root.after(100, self.update_frame)

if __name__ == "__main__":
    root = tk.Tk()
    app = LicensePlateApp(root)
    root.mainloop()
