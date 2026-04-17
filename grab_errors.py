import os

current_path = ["."];
all_errors = []
while current_path != []:
    cur_folder = current_path.pop(0)
    files_folder = os.listdir(cur_folder)

    for file in files_folder:
        full_file = os.path.join(cur_folder, file)

        if file.endswith((".hpp", ".cpp")):
            with open(full_file) as c_file:
                for line in c_file:
                    if "throw" in line:
                        all_errors.append(line.strip())

        elif os.path.isdir(full_file):
            current_path.append(full_file)

for err in all_errors:
    print(err)

