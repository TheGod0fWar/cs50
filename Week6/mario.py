max = 8

number = int(input("Number please"))

while number > max or number < 0:
    number = int(input("Number please"))
number += 1

for i in range(number):
    spaces = " "

    hashtag = "#"

print((number - i) * spaces, hashtag * i, "", hashtag * i)

# TODO:
