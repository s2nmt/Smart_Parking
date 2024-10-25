from pymongo import MongoClient

client = MongoClient('mongodb+srv://20139095:Tuan841826@cluster0.ji13oix.mongodb.net/?retryWrites=true&w=majority&appName=Cluster0')

db = client['mydatabase']
collection = db['listcustomer']
# collection = db['indoor']

collection.delete_many({"plate": "30K-12347"})
# 0x57 0xC4 0xB7 0x3B 0xE1 0xE0 0x11 0x1B
document = {"name": "Phan Tấn Quốcewqe","CODE" : "0xE1 0xE0 0x11 0x1B","licence_plate": "59DB-05813","state":"indoor"}
inserted_document = collection.insert_one(document)
print(f"Inserted Document ID: {inserted_document.inserted_id}")
# collection.update_one(
#     {"licence_plate": "59DB-05813"},  # Điều kiện để xác định tài liệu cần cập nhật
#     {"$set": {"state": "outdoor"}}  # Dữ liệu mới bạn muốn cập nhật
# )
# Query the collection
# query = {}  # You can specify a query if needed, otherwise leave it as an empty dictionary to retrieve all documents
# cursor = collection.find(query)

# # Iterate through the cursor and print documents
# for document in cursor:
#     print(document['_id'])
#     print(document['name'])

client.close()