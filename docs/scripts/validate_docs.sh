#!/bin/bash
# filepath: /Users/ratimics/develop/cgame/docs/scripts/validate_docs.sh

echo "🔍 Validating CGame documentation..."

# Check for required metadata headers
echo "Checking metadata headers..."
find docs -name "*.md" | while read file; do
    if ! grep -q "Document ID\|filepath:" "$file"; then
        echo "❌ Missing metadata: $file"
    fi
done

# Check for broken internal links
echo "Checking internal links..."
find docs -name "*.md" -exec grep -l "\]\(\.\./" {} \; | while read file; do
    grep -o "](\.\.\/[^)]*)" "$file" | while read link; do
        target=$(echo "$link" | sed 's/](//;s/)//')
        full_path="$(dirname "$file")/$target"
        if [ ! -f "$full_path" ]; then
            echo "❌ Broken link in $file: $target"
        fi
    done
done

# Check for source file references
echo "Checking source file references..."
grep -r "\[.*\](.*src/.*\.h)" docs/ | while IFS=: read file link; do
    clean_link=$(echo "$link" | sed -n 's/.*(\([^)]*src\/[^)]*\.h\)).*/\1/p')
    if [ -n "$clean_link" ] && [ ! -f "$clean_link" ]; then
        echo "❌ Dead source link in $file: $clean_link"
    fi
done

# Check for outdated sprint references
echo "Checking sprint references..."
current_sprint=26
find docs -name "*.md" -exec grep -l "Sprint [0-9]" {} \; | while read file; do
    if ! echo "$file" | grep -q "completed\|archive"; then
        old_sprints=$(grep -o "Sprint [0-9][0-9]*" "$file" | grep -v "Sprint $current_sprint" | sort -u)
        if [ -n "$old_sprints" ]; then
            echo "⚠️  Check sprint references in $file: $old_sprints"
        fi
    fi
done

# Check required sections in sprint docs
echo "Checking sprint document structure..."
find docs/sprints/active -name "SPRINT_*.md" | while read file; do
    if ! grep -q "## 🎯 Vision\|## Vision" "$file"; then
        echo "❌ Missing Vision section: $file"
    fi
    if ! grep -q "## 🧪 Testing\|## Testing" "$file"; then
        echo "❌ Missing Testing section: $file"
    fi
    if ! grep -q "## 📊 Success\|## Success" "$file"; then
        echo "❌ Missing Success Criteria: $file"
    fi
done

echo "✅ Documentation validation complete."
