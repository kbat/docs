# Declarative programming
**Declarative** programming tells the program **what** to do, where an **imparative** approach is to tell it **how** to do something.

You walk into a bar.
{% challenge  "Order ten beers for you (and you friends)" %}
You approach the barkeeper and...
{% solution "Imperative" %}
**Imperative (HOW):** ask him to to go over to the glasses, take one, go to the tap and pour a glass. Put it down and do so until he has ten, then give them to you and your friends.
{% solution "Declarative" %}
**Declarative (WHAT):** order ten beers.
However, we have to keep in mind that this indicates that the barkeeper is able to interpret our order in the right way. So what declarative programming essentially does is add an additional high-level layer hiding the underlying control flow(```if...else, switch, for, while```).
{% endchallenge %}

Abstarction also gives the code more reusability in concerns of different data types. See [TDataSource]().
