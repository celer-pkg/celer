// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "../client/petapi.h"
#include "../client/storeapi.h"
#include "../client/userapi.h"

#include <QtCore/qobject.h>
#include <QtCore/qprocess.h>
#include <QtGui/qimage.h>
#include <QtNetwork/qnetworkrequestfactory.h>
#include <QtNetwork/qrestaccessmanager.h>
#include <QtTest/qtest.h>

using namespace Qt::StringLiterals;
using namespace QtOpenApiCommon;

namespace QtOpenAPI {
const int REPLY_OK = 200;

static QProcess serverProcess;
void startServerProcess()
{
    serverProcess.setWorkingDirectory(SERVER_FOLDER);
    serverProcess.start(SERVER_PATH);
    if (!serverProcess.waitForStarted()) {
        qFatal() << "Couldn't start the server: " << serverProcess.errorString();
        exit(EXIT_FAILURE);
    }
    // give the process some time to properly start up the server
    QThread::currentThread()->msleep(1000);
}

class PetApiTests : public QObject {
    Q_OBJECT

    Pet createRandomPet(const QString &status = "sold", const QString &name = "monster");
    void connectAddPetApi(PetApi *petApi, bool &petCreated);

private Q_SLOTS:
    void initTestCase()
    {
        if (serverProcess.state() != QProcess::ProcessState::Running)
            startServerProcess();
    }
    void findPetsByStatusTest();
    void createAndGetPetTest();
    void updatePetTest();
    void updatePetWithFormTest();
    void deleteCreatedPetByBearerTest();
    void deleteCreatedPetNoBearerTest();
    void uploadPetFileTest();
    void mixedApiCallsTest();
    void getFilesFromServerTest();
    void sslConfigurationTest();
    void setNoBasicLoginAndPasswordTest();
    void getPatientPetsTest();
    void cleanupTestCase();
};

const QString user("User1");
const QString password("1234");

class PetApiInheritageTest: public PetApi
{
public:
    PetApiInheritageTest(QObject *parent = nullptr)
        : PetApi(parent){}

    bool m_testCheck = false;
private:
    // Template operation function will call this overriten method
    void addPetWithDataImpl(const Pet &Pet, const QObject *context, QtPrivate::QSlotObjectBase *slot) override
    {
        m_testCheck = true;
        PetApi::addPetWithDataImpl(Pet, context, slot);
    }
};

Pet PetApiTests::createRandomPet(const QString &status, const QString &name) {
    Pet pet;
    const QList<QString> urls = {"https://text.com"};
    const qint64 id = static_cast<long long>(rand());
    pet.setName(name);
    pet.setId(id);
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
    pet.setStatus(status);
QT_WARNING_POP
    pet.setAge(10);
    pet.setPatience(9);
    pet.setPhotoUrls(urls);
    // Server sends us category: {} (even before our changes it always sent it).
    // Before we ignored such data, but now we distinguish 'no field' and 'empty field'.
    // But our server doesn't distinguish it. So we need to add Category("{}")
    // to fix client side tests.
    pet.setCategory(Category("{}"));
    return pet;
}

void PetApiTests::findPetsByStatusTest() {
    PetApi api;
    bool petFound = false;
    bool petCreated = false;
    api.setUsername(user);
    api.setPassword(password);
    api.setApiKey("api_key","special-key");

    const Pet randomPet = createRandomPet();
    Pet availablePet = createRandomPet("available", "avaialble_pet");
    availablePet.setId(1111);
    Pet sold_pet = createRandomPet("sold", "sold_pet");
    sold_pet.setId(2222);

    connectAddPetApi(&api, petCreated);
    api.addPet(randomPet);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petCreated, true, 5000);

    petCreated = false;
    api.addPet(availablePet);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petCreated, true, 5000);

    petCreated = false;
    api.addPet(sold_pet);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petCreated, true, 5000);

    connect(&api, &PetApi::findPetsByStatusFinished,
            this, [&](const QList<Pet> &pets) {
        petFound = true;
        QVERIFY(!pets.isEmpty());
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
        for (const Pet &pet: pets) {
            const QString statusString = pet.getStatusValue().asJson();
            qDebug() << "Pet id = " << pet.getIdValue() << "status = " << statusString;
            QVERIFY(statusString == "available"_L1 || statusString == "sold"_L1);
        }
QT_WARNING_POP
    });
    connect(&api, &PetApi::findPetsByStatusErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });

    api.findPetsByStatus({"available", "sold"});
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petFound, true, 5000);
}

void connectPetByIdApi(PetApi *petApi, bool &petFetched, Pet &petToCheck)
{
    QObject::connect(petApi, &PetApi::getPetByIdFinished,
                     petApi, [&petFetched, &petToCheck](const Pet &summary) {
        // pet created
        petFetched = true;
        petToCheck = summary;
    });
    QObject::connect(petApi, &PetApi::getPetByIdErrorOccurred,
                     petApi, [&](QNetworkReply::NetworkError, const QString &errorStr) {
                         qDebug() << "Error happened while issuing request : " << errorStr;
                     });
}

void PetApiTests::createAndGetPetTest() {
    PetApi api;
    api.setUsername(user);
    api.setPassword(password);
    api.setApiKey("api_key","special-key");
    bool petCreated = false;

    const QString petName("Exclusive name");
    Pet pet = createRandomPet("available", petName);
    qint64 id = pet.getIdValue();

    connect(&api, &PetApi::addPetFinished, this, [&](const Pet &summary) {
        // pet created
        petCreated = true;
        QCOMPARE(pet, summary);
    });
    connect(&api, &PetApi::addPetErrorOccurred, this,
            [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });
    api.addPet(pet);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petCreated, true, 14000);

    bool petFetched = false;
    Pet petToCheck;
    connectPetByIdApi(&api, petFetched, petToCheck);
    api.getPetById(id);

    QTRY_COMPARE_EQ_WITH_TIMEOUT(petFetched, true, 14000);
    QVERIFY2(petToCheck.getNameValue().compare(petName) == 0, "pet isn't found.");

    PetApiInheritageTest mockedApi;
    mockedApi.setUsername(user);
    mockedApi.setPassword(password);
    mockedApi.setApiKey("api_key","special-key");
    petCreated = false;
    mockedApi.addPet(pet, this, [&](const QRestReply &reply, const Pet &newPet) {
        if (!(petCreated = reply.isSuccess())) {
            qWarning() << "Not successful" << reply.errorString();
        } else {
            QCOMPARE(pet, newPet);
        }
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petCreated, true, 14000);
    QVERIFY2(mockedApi.m_testCheck, "template isn't called with re-implemented impl()");
}

void PetApiTests::updatePetTest() {
    PetApi api;
    api.setUsername(user);
    api.setPassword(password);
    Pet pet = createRandomPet();
    Pet petToCheck;
    const qint64 id = pet.getIdValue();
    bool petAdded = false;

    connect(&api, &PetApi::addPetFinished, this, [&](const Pet &summary) {
        petAdded = true;
        QCOMPARE(pet, summary);
    });
    connect(&api, &PetApi::addPetErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });
    // create pet
    api.addPet(pet);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petAdded, true, 5000);

    // fetch it
    bool petFetched = false;
    connectPetByIdApi(&api, petFetched, petToCheck);
    // create pet
    api.getPetById(id);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petFetched, true, 5000);

    // update it
    bool petUpdated = false;
    connect(&api, &PetApi::updatePetFinished, this, [&](const Pet &summary) {
        petUpdated = true;
        pet = summary;
    });
    connect(&api, &PetApi::updatePetErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
    // update pet
    petToCheck.setStatus(QString("scary"));
QT_WARNING_POP
    api.updatePet(petToCheck);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petUpdated, true, 5000);

    // api already connected above by connectPetByIdApi
    petFetched = false;
    api.getPetById(id);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petFetched, true, 5000);
    QVERIFY2(pet.getIdValue() == petToCheck.getIdValue(), "pet isn't found");
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
    QCOMPARE_EQ(pet.getStatusValue().asJson(), petToCheck.getStatusValue().asJson());
QT_WARNING_POP
}

void PetApiTests::updatePetWithFormTest() {
    PetApi api;
    api.setUsername(user);
    api.setPassword(password);
    Pet pet = createRandomPet();
    Pet petToCheck;
    const qint64 id = pet.getIdValue();

    // create pet
    bool petAdded = false;
    connect(&api, &PetApi::addPetFinished, this, [&](const Pet &summary) {
        petAdded = true;
        QCOMPARE(pet, summary);
    });
    connect(&api, &PetApi::addPetErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });

    api.addPet(pet);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petAdded, true, 5000);

    // fetch it
    bool petFetched = false;
    connectPetByIdApi(&api, petFetched, petToCheck);
    api.getPetById(id);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petFetched, true, 5000);

    // update it
    bool petUpdated = false;
    connect(&api, &PetApi::updatePetWithFormFinished, this, [&]() {
        petUpdated = true;
    });
    connect(&api, &PetApi::updatePetWithFormErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });

    QString name("gorilla");
    api.updatePetWithForm(id, name);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petUpdated, true, 5000);

    // fetch it
    petFetched = false;
    api.getPetById(id);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petFetched, true, 5000);
}

// Bearer Authentication:
// see https://swagger.io/docs/specification/v3_0/authentication/bearer-authentication
void PetApiTests::deleteCreatedPetByBearerTest()
{
    PetApi api;
    api.setUsername(user);
    api.setPassword(password);
    api.setBearerToken("BEARER-TOKEN");
    Pet pet = createRandomPet();
    const qint64 id = pet.getIdValue();

    // create pet
    bool petAdded = false;
    connect(&api, &PetApi::addPetFinished, this, [&](const Pet &summary) {
        petAdded = true;
        QCOMPARE(pet, summary);
    });
    connect(&api, &PetApi::addPetErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });

    api.addPet(pet);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petAdded, true, 5000);

    // delete created pet
    bool petDeleted = false;
    connect(&api, &PetApi::deletePetFinished, this, [&]() {
        petDeleted = true;
    });
    connect(&api, &PetApi::deletePetErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });

    api.deletePet(id);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petDeleted, true, 5000);
}

// Bearer Authentication:
// see https://swagger.io/docs/specification/v3_0/authentication/bearer-authentication
void PetApiTests::deleteCreatedPetNoBearerTest()
{
    PetApi apiNoBearer;
    // delete created pet
    bool petNotDeleted = false;
    int errorOccurredCounter = 0;
    connect(&apiNoBearer, &PetApi::deletePetFinished, this, [&]() {
        qDebug() << "deletePetFinished: No error happened";
    });
    connect(&apiNoBearer, &PetApi::deletePetErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
                qDebug() << "deletePetErrorOccurred: expected error happened while issuing the request : " << errorStr;
                petNotDeleted = true;
                errorOccurredCounter++;
            });

    apiNoBearer.deletePet(0);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petNotDeleted, true, 5000);
    // The slot is called only once, no duplication anymore
    QTRY_COMPARE_EQ_WITH_TIMEOUT(errorOccurredCounter, 1, 5000);
}

void PetApiTests::uploadPetFileTest()
{
    PetApi api;
    api.setUsername(user);
    api.setPassword(password);
    Pet pet = createRandomPet();
    const qint64 id = pet.getIdValue();

    // create pet
    bool petAdded = false;
    connect(&api, &PetApi::addPetFinished, this, [&](const Pet &summary) {
        petAdded = true;
        QCOMPARE(pet, summary);
    });
    connect(&api, &PetApi::addPetErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });

    api.addPet(pet);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petAdded, true, 5000);

    // delete created pet
    bool petFileUploaded = false;
    QString type, message;
    qint32 code = -100;
    connect(&api, &PetApi::uploadFileFinished, this, [&](const ApiResponse &response) {
        petFileUploaded = true;
        type = response.getTypeValue();
        code = response.getCodeValue();
        message = response.getMessageValue();
        qWarning() << type << code << message;
    });
    connect(&api, &PetApi::uploadFileErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });

    QOAIHttpFileElement fileElement(":/file-for-uploading.txt"_L1);
    fileElement.setVariableName("Variable=100"_L1);
    fileElement.setMimeType("txt"_L1);

    api.uploadFile(id, QString("metadata-info"), fileElement);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petFileUploaded, true, 5000);
    QVERIFY2(code != -100, "didn't finish within timeout");
}

void PetApiTests::connectAddPetApi(PetApi *petApi, bool &petCreated)
{
    QObject::connect(petApi, &PetApi::addPetFinished, this, [&petCreated]() {
        // pet created
        petCreated = true;
    });
    QObject::connect(petApi, &PetApi::addPetErrorOccurred,
                     this, [&] (QNetworkReply::NetworkError, const QString &errorStr) {
                         qDebug() << "Error happened while issuing request : " << errorStr;
                     });
}

void PetApiTests::mixedApiCallsTest()
{
    QNetworkAccessManager manager;
    QRestAccessManager restManager(&manager);
    QNetworkRequestFactory factory;

    PetApi api1, api2;
    api1.setUsername(user);
    api1.setPassword(password);
    api2.setUsername(user);
    api2.setPassword(password);
    StoreApi apiStore;
    UserApi apiUser;

    // test resource re-setting
    apiStore.setRestAccessManager(&restManager);
    apiStore.setNetworkRequestFactory(factory);
    apiUser.setRestAccessManager(&restManager);
    apiUser.setNetworkRequestFactory(factory);
    Pet pet1 = createRandomPet();
    Pet pet2 = createRandomPet();

    api1.setApiKey("api_key","special-key");
    bool petCreated = false;
    connectAddPetApi(&api1, petCreated);
    connectAddPetApi(&api2, petCreated);

    api1.addPet(pet1);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petCreated, true, 14000);

    petCreated = false;
    api2.addPet(pet2);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petCreated, true, 14000);

    bool inventoryFetched = false;
    connect(&apiStore, &StoreApi::getInventoryFinished,
            this, [&](const QMap<QString, qint32> &status) {
        inventoryFetched = true;
        for (const auto &key : status.keys()) {
            qDebug() << (key) << " Quantities " << status.value(key);
        }
    });
    connect(&apiStore, &StoreApi::getInventoryErrorOccurred,
            this, [&] (QNetworkReply::NetworkError, const QString &errorStr) {
                qDebug() << "Error happened while issuing request : " << errorStr;
            });

    apiStore.getInventory();
    QTRY_COMPARE_EQ_WITH_TIMEOUT(inventoryFetched, true, 14000);

    bool userLoggedOut = false;
    connect(&apiUser, &UserApi::logoutUserFinished, this, [&]() {
        userLoggedOut = true;
    });
    connect(&apiUser, &UserApi::logoutUserErrorOccurred,
            this, [&] (QNetworkReply::NetworkError, const QString &errorStr) {
                qDebug() << "Error happened while issuing request : " << errorStr;
            });

    apiUser.logoutUser(QJsonValue("johndoe"));
    QTRY_COMPARE_EQ_WITH_TIMEOUT(userLoggedOut, true, 14000);
}

void PetApiTests::getFilesFromServerTest()
{
    PetApi api;
    api.setUsername(user);
    api.setPassword(password);
    Pet pet = createRandomPet();
    const qint64 id = pet.getIdValue();

    // create pet
    bool petAdded = false;
    connect(&api, &PetApi::addPetFinished, this, [&](const Pet &summary) {
        petAdded = true;
        QCOMPARE(pet, summary);
    });
    connect(&api, &PetApi::addPetErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });

    api.addPet(pet);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petAdded, true, 14000);

     // get pet json info file
    bool petFileDownloaded = false;
    connect(&api, &PetApi::getJsonFileFinished,
            this, [&](const QOAIHttpFileElement &summary) {
        petFileDownloaded = true;
                QCOMPARE("response.json", summary.requestFilename());
        QJsonObject fileContent = summary.asJsonValue().toObject();
        QCOMPARE(fileContent.value("file-name").toString(), QString("Hi, I am a response!"));
        QCOMPARE(fileContent.value("value").toInt(), 81);
    });
    connect(&api, &PetApi::getJsonFileErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
                qDebug() << "Error happened while issuing request : " << errorStr;
            });

    api.getJsonFile(id);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petFileDownloaded, true, 5000);

    // get PNG file
    bool petPngDownloaded = false;
    connect(&api, &PetApi::findPetsImageByIdFinished, this, [&](const QString &summary) {
        petPngDownloaded = true;
        // test file size
        QCOMPARE(summary.size(), 8868);
        // test we can load it normally into QImage object
        QVERIFY2(!QImage::fromData(QByteArray::fromBase64(summary.toUtf8()), "png").isNull(), "Image isn't loaded.");
    });
    connect(&api, &PetApi::findPetsImageByIdErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });

    api.findPetsImageById(id);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petPngDownloaded, true, 5000);
}

void PetApiTests::sslConfigurationTest()
{
#if QT_CONFIG(ssl)
    PetApi api;
    QNetworkRequestFactory factory;
    auto config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1_2OrLater);
    api.setNetworkRequestFactory(factory);
    api.setSslConfiguration(config);
    QCOMPARE(api.sslConfiguration(), config);
#else
    QSKIP("Skipping SSL test, not supported by current Qt configuration");
#endif // ssl
}

// Basic login:password - headers authentication,
// see https://swagger.io/docs/specification/v3_0/authentication/basic-authentication
// Testing if no data provided
void PetApiTests::setNoBasicLoginAndPasswordTest()
{
    PetApi apiNoData;
    Pet pet = createRandomPet();
    bool petNotAdded = false;
    connect(&apiNoData, &PetApi::addPetFinished, this, [&]() {
        qDebug() << "addPetFinished: no error happened.";
    });
    connect(&apiNoData, &PetApi::addPetErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        petNotAdded = true;
        qDebug() << "addPetErrorOccurred: expected error happened while issuing the request : " << errorStr;
    });

    apiNoData.addPet(pet);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petNotAdded, true, 5000);
}

void PetApiTests::getPatientPetsTest()
{
    PetApi api;
    api.setUsername(user);
    api.setPassword(password);

    Pet pet1 = createRandomPet("sold", "Poops_Baraboops");
    pet1.setAge(100); // oddly old cat though

    Pet pet2 = createRandomPet("sold", "TheThing");
    pet2.setAge(33); // also oddly old cat though
    pet2.setPatience(0);

    // create pet1
    bool operationStatus = false;
    api.addPet(pet1, this, [&](const QRestReply &reply, const Pet &summary) {
        if (!(operationStatus = reply.isSuccess()))
            qDebug() << "Error happened while issuing request : " << reply.errorString();

        QVERIFY(operationStatus);
        QCOMPARE(summary, pet1);

    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(operationStatus, true, 14000);

    // create pet2
    operationStatus = false;
    api.addPet(pet2, this, [&](const QRestReply &reply, const Pet &summary) {
        if (!(operationStatus = reply.isSuccess()))
            qDebug() << "Error happened while issuing request : " << reply.errorString();

        QVERIFY(operationStatus);
        QCOMPARE(summary, pet2);

    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(operationStatus, true, 14000);

    // found pets with same data
    QList<qint32> petData;
    petData.append(pet1.getAgeValue());
    petData.append(pet1.getPatienceValue());
    petData.append(pet2.getAgeValue());
    petData.append(pet2.getPatienceValue());
    operationStatus = false;
    api.findPetsByAgeAndPatience(petData, this, [&](const QRestReply &reply, const QList<Pet> &summary) {
        if (!(operationStatus = reply.isSuccess()))
            qDebug() << "Error happened while issuing request : " << reply.errorString();

        QVERIFY(operationStatus);
        QCOMPARE(REPLY_OK, reply.httpStatus());
        QVERIFY(summary.count() == 2);
        const std::array<Pet, 2> expectedPets = {pet1, pet2};
        QVERIFY(std::is_permutation(summary.cbegin(), summary.cend(),
                                    expectedPets.cbegin(), expectedPets.cend()));

    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(operationStatus, true, 14000);
}

void PetApiTests::cleanupTestCase()
{
    if (serverProcess.state() == QProcess::ProcessState::Running) {
        serverProcess.kill();
        serverProcess.waitForFinished();
    }
}

} // QtOpenAPI

QTEST_MAIN(QtOpenAPI::PetApiTests)
#include "tst_petapitests.moc"
