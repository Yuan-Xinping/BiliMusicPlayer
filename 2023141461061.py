listA = ["hello", 'seven', ["mon", ["h", "kelly"], 'all'], 123, 446]

for index,item in enumerate(listA):
    if(item==123):
        print(f"{index}")


def conver_all_to_upper(target_list):
    for index,item in enumerate(target_list):
        if isinstance(item, str):
            target_list[index]=item.upper()
        elif isinstance(item, list):
            conver_all_to_upper(item)

conver_all_to_upper(listA)
print(listA)
