var particle = null;

var particleAccessToken = null;
var particleDeviceId = null;
var particleEventName = null;

var User = null;

var learning = false;

function init() {
    settingsCollection = firebase.firestore().collection('settings');
    dataCollection = firebase.firestore().collection('data');

    settingsCollection.get().then(function(coll) {
        if (coll) {
            coll.forEach(function(doc) {
                var docId = doc.id;
                var valInfo = doc.data();

                if (valInfo.value) {
                    switch (docId) {
                        case 'access_token':
                            particleAccessToken = valInfo.value;
                            break;
                        case 'device_id':
                            particleDeviceId = valInfo.value;
                            break;
                        case 'event_name':
                            particleEventName = valInfo.value;
                            break;
                    }
                }
            });

            setupEventStream();
        }
    });

    dataCollection.onSnapshot(function(snapshot) {
        $('#executeFunction').html('');
        snapshot.docs.forEach(function(doc) {
            var docData = doc.data();

            $('#executeFunction').append(`
                <option value='${JSON.stringify(docData)}'>${doc.id}</option>
            `);
        });
    });
}

function setupEventStream() {
    particle.getEventStream({
        deviceId: 'mine',
        name: particleEventName,
        auth: particleAccessToken
    }).then(function(stream) {
        stream.on('event', function(data) {
            var decodedData = decodeObject(data.data);
            data.data = decodedData;
            $('table > tbody').prepend(`
                <tr>
                    <td>${data.name}</td>
                    <td><pre class='table-log'>${JSON.stringify(data, null, 4)}</pre></td>
                </tr>
            `);
            console.log('Event: ', data);

            handleData(data);
        });
    });
}

function handleData(data) {
    switch (data.data.subEvent) {
        case 'startLearning':
            learning = true;
            break;
        case 'stopLearning':
            learning = false;
            $('#saveModal').modal('show');
            break;
        case 'learning':
            if (learning) {
                $('#sequenceFormGroups').append(`
                    <div class="form-group">
                        <label for="code${$(this).children().length}">Name for Code ${"0x" + data.data.code}</label>
                        <input type="text" class="form-control" id="code${$(this).children().length}" placeholder="Power" data-code="${"0x" + data.data.code}">
                    </div>
                `);
            }
            break;
    }
}

$(function() {
    particle = new Particle();

    firebase.auth().signInAnonymouslyAndRetrieveData().then(function(user) {
        User = user;
        init();
    }).catch(function(err) {
        console.error(err);
    });

    $('#executeCode').click(function(event) {
        event.preventDefault();

        var data = JSON.parse($('#executeFunction').val());

        particle.publishEvent({
            name: particleEventName,
            data: encodeObject({
                subEvent: 'executeFunction',
                codes: data.order
            }),
            auth: particleAccessToken
        }).then(function(data) {
            if (data.body.ok) {
                console.log('Event published succesfully');
            }
        }).catch(function(err) {
            console.log('Failed to publish event: ' + err);
        });
    });

    $('#saveCodes').click(function(event) {
        event.preventDefault();

        var realName = "";
        var sequence = {};
        var order = [];
        var codeOut = ""

        $('#sequenceFormGroups').find('input').each(function() {
            var code = $(this).data('code');
            var name = $(this).val() || code;

            if (!code) {
                realName = name;
                return;
            }

            order.push(code);
            sequence[name] = code;
            codeOut = code;
        });

        dataCollection.doc(realName || codeOut).set({
            sequence,
            order
        });

        $('#saveModal').modal('hide');
        $('#sequenceFormGroups').html(`
            <div class="form-group">
                <label for="sequenceFunction">Name for the function of this sequence</label>
                <input type="text" class="form-control" id="sequenceFunction" placeholder="Turn on and change input">
            </div>
        `);
    });

    $('#showModal').click(function(event) {
        event.preventDefault();

        $('#saveModal').modal('show');
    });

    $('#startLearning').click(function(event) {
        event.preventDefault();

        if (!particleAccessToken || !$(this).hasClass('btn-primary')) {
            return;
        }

        $(this).toggleClass('btn-primary btn-success');
        $('#stopLearning').toggleClass('btn-danger btn-primary');
        $(this).text('Started Learning');
        $('#stopLearning').text('Stop Learning');

        particle.publishEvent({
            name: particleEventName,
            data: encodeObject({
                subEvent: 'startLearning'
            }),
            auth: particleAccessToken
        }).then(function(data) {
            if (data.body.ok) {
                console.log('Event published succesfully');
            }
        }).catch(function(err) {
            console.log('Failed to publish event: ' + err);
        });
    });

    $('#stopLearning').click(function(event) {
        event.preventDefault();

        if (!particleAccessToken || !$(this).hasClass('btn-danger')) {
            return;
        }

        $('#startLearning').toggleClass('btn-primary btn-success');
        $('#startLearning').text('Start Learning');
        $(this).toggleClass('btn-danger btn-primary');
        $(this).text('Stopped Learning');

        particle.publishEvent({
            name: particleEventName,
            data: encodeObject({
                subEvent: 'stopLearning'
            }),
            auth: particleAccessToken
        }).then(function(data) {
            if (data.body.ok) {
                console.log('Event published succesfully');
            }
        }).catch(function(err) {
            console.log('Failed to publish event: ' + err);
        });
    });
});

function encodeObject(obj) {
    var encodedData = '';

    for (var key in obj) {
        encodedData += key + ':' + obj[key] + ';';
    }

    return encodedData;
}

function decodeObject(str) {
    var decodedData = {};
    var data = str.split(';');

    for (var group of data) {
        if (group) {
            var groupData = group.split(':');

            decodedData[groupData[0]] = groupData[1];
        }
    }

    return decodedData;
}