#!/bin/bash

SUCCEEDED=
FAILED=

for c in *.yaml; do
  esphome -q config ${c}
  if [ $? -ne 0 ]; then
    if [ -z "$FAILED" ]; then
      FAILED=${c}
    else
      FAILED=${FAILED}" "${c}
    fi
  else
    if [ -z "$SUCCEEDED" ]; then
      SUCCEEDED=${c}
    else
      SUCCEEDED=${SUCCEEDED}" "${c}
    fi
  fi
done

echo ""
echo "Succeeded: "
for s in ${SUCCEEDED}; do
  echo "    "${s}
done
echo ""
echo "Failed: "
for f in ${FAILED}; do
  echo "    "${f}
done
