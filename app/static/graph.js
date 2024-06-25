document.addEventListener('DOMContentLoaded', function () {
    const tempCtx = document.getElementById('tempChart').getContext('2d');
    const humiCtx = document.getElementById('humiChart').getContext('2d');

    const tempChart = new Chart(tempCtx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Temperature',
                data: [],
                borderColor: 'rgba(255, 99, 132, 1)',
                backgroundColor: 'rgba(255, 99, 132, 0.2)',
                fill: true,
                tension: 0.1
            }]
        },
        options: {
            scales: {
                x: {
                    type: 'time',
                    time: {
                        unit: 'minute'
                    },
                    title: {
                        display: true,
                        text: 'Time'
                    }
                },
                y: {
                    title: {
                        display: true,
                        text: 'Temperature (°C)'
                    }
                }
            }
        }
    });

    const humiChart = new Chart(humiCtx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Humidity',
                data: [],
                borderColor: 'rgba(54, 162, 235, 1)',
                backgroundColor: 'rgba(54, 162, 235, 0.2)',
                fill: true,
                tension: 0.1
            }]
        },
        options: {
            scales: {
                x: {
                    type: 'time',
                    time: {
                        unit: 'minute'
                    },
                    title: {
                        display: true,
                        text: 'Time'
                    }
                },
                y: {
                    title: {
                        display: true,
                        text: 'Humidity (%)'
                    }
                }
            }
        }
    });

    function loadNewData() {
        $.ajax({
            url: '/get_data',
            type: 'GET',
            success: function (data) {
                const tempData = [];
                const humiData = [];
                const labels = [];

                data.forEach(function (item) {
                    const time = new Date(item.time);
                    labels.push(time);
                    tempData.push(item.temperature);
                    humiData.push(item.humidity);
                });

                tempChart.data.labels = labels;
                tempChart.data.datasets[0].data = tempData;
                tempChart.update();

                humiChart.data.labels = labels;
                humiChart.data.datasets[0].data = humiData;
                humiChart.update();
            },
            error: function () {
                console.log('Error loading new data');
            }
        });
    }

    setInterval(loadNewData, 10000);
    loadNewData(); // Load data immediately on page load
});