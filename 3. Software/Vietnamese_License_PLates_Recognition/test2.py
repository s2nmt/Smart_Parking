import requests
import urllib.parse

# Construct the URL
url = "http://13.228.24.205:3000/datauser"

# Send the GET request
response = requests.get(url)
counter = 0
if response.status_code == 200:
    for item in response.json():
        print(item.get('state'))
        if(item.get('state') == "indoor"):
            counter = counter + 1
    print(counter)
else:
    print("Failed to retrieve data. Status code:", response.status_code)
