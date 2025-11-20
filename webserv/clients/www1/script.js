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


function isPalendrome(word)
{
    const reverseWord = word
        .split-
}

console.log(reverse("Salut"));