s = 'huge text huge problems \n'
s = s * 300
with open("build/test.txt", "w") as text_file:
    text_file.write(s)
