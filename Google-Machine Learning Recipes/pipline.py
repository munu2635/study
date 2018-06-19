# import a dataset
from sklearn import datasets
iris = datasets.load_iris()


# Feature(f(X)), rabel(y) ==>  ( f(x) = y )
x = iris.data
y = iris.target

# 반을 테스트 데이터로 반은 트레이닝 데이터로
from sklearn.cross_validation import train_test_split
x_train, x_test, y_train, y_test = train_test_split(x, y, test_size = .5)

'''
# 의사 결정 트리사용 방법
from sklearn import tree
my_classifier = tree.DecisionTreeClassifier()
'''

# Use KNearestNeighbors
from sklearn.neighbors import KNeighborsClassifier
my_classifier = KNeighborsClassifier()


my_classifier.fit(x_train, y_train)

predictions = my_classifier.predict(x_test)
# print ( predictions )

from sklearn.metrics import accuracy_score

print ( accuracy_score(y_test, predictions) )
