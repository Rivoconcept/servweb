document.getElementById('deleteBtn').addEventListener('click', function()
{
    let path = document.getElementById('filePath').value.trim();
    if (!path) {
        alert('Please enter a file path.');
        return;
    }

    fetch('/' + path, {
        method: 'DELETE'
    })
    .then(function(response) {
        if (response.status === 204) {
            document.getElementById('deleteResult').textContent = "File successfully deleted !";
        } else if (response.status === 404) {
            document.getElementById('deleteResult').textContent = "File not found (404).";
        } else if (response.status === 403) {
            document.getElementById('deleteResult').textContent = "Access denied (403).";
        } else {
            document.getElementById('deleteResult').textContent = "Error server (" + response.status + ").";
        }
    })
    .catch(function(err) {
        document.getElementById('deleteResult').textContent = "Error : " + err;
    });
});


const sentence = `Comment faire les choses correctements sans savoir des choses comment ni les choses à faire`


const words = sentence.split(' ')

for (let word of words)
{
    let n = 0;
    for (let i in words)
    {
        if (word.toLowerCase() === words[i].toLocaleLowerCase())
        {
            n++;
        }
    }
    console.log(`${word} (${n})`)
}
/*function getRandomInt(max)
{
    return (Math.floor(Math.random() * (max + 1)));
}

const intRandom = getRandomInt(10);
console.log(intRandom);

function isRight(n)
{
    return (intRandom === n);
}

function guess()
{
    const number = parseInt(prompt("Enter a number:"), 10);
    return (isRight(number));
}

for (let i = 0; i < 3; i++)
{
    if (guess())
    {
        console.log("bravo")
        break;
    } else if (i === 2)
    {
        console.log("you lose");
    }
}*/


/*function isPremier(n)
{
    if (n < 2)
    {
        return (false);
    }
    for (let i = n - 1; i > 1; i--)
    {
        if (n % i === 0)
        {
            return (false)
        }
    }
    return (true)
}

console.log('0', isPremier(0));
console.log('11', isPremier(11));
console.log('12', isPremier(12));*/


/*const students = [
    {
        name : 'John',
        notes: [1, 20, 18, 19, 12]
    },
    {
        name : 'Jane',
        notes: [17, 18, 20, 13, 15]
    },
    {
        name : 'Sophie',
        notes: [17, 12, 14, 15, 13]
    },
    {
        name : 'Marc',
        notes: [2, 3, 5, 8, 9]
    },
    {
        name : 'Manon',
        notes: [18, 17, 18, 19, 12]
    },
]

const calculMoyenne = (notes) => {
    let sum = 0;
    for (let note of notes)
    {
        sum += note
    }
    return (sum / notes.length)
}


for (let student of students)
{
    student.moyenne = calculMoyenne(student.notes);
    student.best = Math.max(...student.notes)
    student.worst = Math.min(...student.notes)
}


function compareStudents(a, b)
{
    return (b.moyenne - a.moyenne)
}

students.sort(compareStudents)

const formatStudent = (student) => {
    return (`${student.name} avec une moyenne de ${student.moyenne}. Sa meilleure note ${student.best} et sa mauvaise note ${student.worst}`)
}

console.log('Top 3 des étudiants')

for (let i = 0; i < 3; i++)
{
    console.log(`${i}: ${formatStudent(students[i])}`)
}*/
